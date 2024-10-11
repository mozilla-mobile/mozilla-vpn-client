/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <QLocalSocket>
#include <QObject>
#include <QProcess>
#include <QTcpServer>
#include <QTcpSocket>

class MockWorker final : public QObject {
 Q_OBJECT
 public:
  MockWorker(QTcpSocket* s, const QString& name, QObject* parent = nullptr);

 private:
  void pipeRead();
  void socketRead();
  void connected();

  const QString m_name;
  QLocalSocket* m_pipe = nullptr;
  QTcpSocket* m_socket;
};
#include "curlpipe.moc"

MockWorker::MockWorker(QTcpSocket* s, const QString& name, QObject* parent)
    : QObject(parent), m_name(name), m_socket(s) {
  s->setParent(this);

  qInfo() << "Connection started:" << m_socket->peerAddress().toString();
  m_pipe = new QLocalSocket(this);
  connect(m_pipe, &QLocalSocket::connected, this, [&]() {
    connect(m_pipe, &QLocalSocket::readyRead, this, &MockWorker::pipeRead);
    connect(m_socket, &QLocalSocket::readyRead, this, &MockWorker::socketRead);
  });

  connect(m_pipe, &QLocalSocket::disconnected, this, &QObject::deleteLater);
  connect(m_pipe, &QLocalSocket::errorOccurred, this, &QObject::deleteLater);
  connect(m_socket, &QTcpSocket::disconnected, this, &QObject::deleteLater);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &QObject::deleteLater);

  m_pipe->connectToServer(m_name);
}

void MockWorker::connected() {
}

void MockWorker::pipeRead() {
  while (m_pipe->bytesAvailable()) {
    QByteArray data = m_pipe->read(4096);
    m_socket->write(data);
  }
}

void MockWorker::socketRead() {
  while (m_socket->bytesAvailable()) {
    QByteArray data = m_socket->read(4096);
    m_pipe->write(data);
  }
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QString pipeName = "\\\\.\\pipe\\chrome.vpn.proxy";
  QStringList curlArgs = app.arguments().mid(1);
  auto i = curlArgs.begin();
  while (i != curlArgs.end()) {
    if (i->startsWith("--pipe=")) {
      pipeName = i->mid(7);
      i = curlArgs.erase(i);
    } else if (*i == "--pipe") {
      i = curlArgs.erase(i);
      if (i == curlArgs.end()) {
        qFatal() << "Argument --pipe requires a value.";
      }
      pipeName = *i;
      i = curlArgs.erase(i);
    } else {
      i++;
    }
  }

  // Setup a QTcpServer to broker connection to the named pipes.
  QTcpServer server;
  QObject::connect(&server, &QTcpServer::newConnection,
                   [&]() {
                     while (server.hasPendingConnections()) {
                       auto* s = server.nextPendingConnection();
                       if (!s) break;
                       auto* worker = new MockWorker(s, pipeName);
                     }
                   });
  server.listen(QHostAddress::LocalHost);
  QString proxyUrl = QString("socks5://127.0.0.1:%1").arg(server.serverPort());

  // Launch curl with all the arguments we have been given.
  QProcess curl;
  curl.setProgram("curl");
  curl.setArguments(QStringList({"--proxy", proxyUrl}) + curlArgs);
  curl.setProcessChannelMode(QProcess::ForwardedChannels);

  // Start curl and run the event loop until it exits.
  QObject::connect(&curl, &QProcess::finished, &app, &QCoreApplication::exit);
  curl.start();
  app.exec();
}
