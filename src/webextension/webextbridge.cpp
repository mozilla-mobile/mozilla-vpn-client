/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include "webextbridge.h"
#include "webexthandler.h"

const QStringList ALLOW_LISTED_WEBEXTENSIONS = {
    "@testpilot-containers",
    "vpn@mozilla.com"
};

constexpr const quint16 VPN_WEBEXTENSION_PORT = 8754;

constexpr const int BRIDGE_RETRY_DELAY = 500;

WebExtBridge::WebExtBridge(quint16 port, QObject* parent)
    : QObject(parent), m_port(port) {

  connect(&m_socket, &QAbstractSocket::stateChanged, this,
          &WebExtBridge::stateChanged);

  connect(&m_socket, &QAbstractSocket::errorOccurred, this,
          &WebExtBridge::errorOccurred);

  connect(&m_socket, &QIODevice::bytesWritten, this,
          &WebExtBridge::tryPushData);

  connect(&m_retryTimer, &QTimer::timeout, this,
          &WebExtBridge::retryConnection);

  // Upon disconnection - attempt to retry after a short delay.
  m_retryTimer.setSingleShot(true);
  connect(&m_socket, &QAbstractSocket::stateChanged, &m_retryTimer,
          [this](QAbstractSocket::SocketState state) {
            if (state == QAbstractSocket::UnconnectedState) {
              m_retryTimer.start(BRIDGE_RETRY_DELAY);
            }
          });

  // Immediately try to establish a connection.
  retryConnection();
}

void WebExtBridge::stateChanged(QAbstractSocket::SocketState state) {
  //qDebug() << "Web extension socket state:" << state;
  if (state == QAbstractSocket::ConnectedState) {
    Q_ASSERT(m_reader == nullptr);
    m_reader = new WebExtReader(&m_socket, this);
    connect(&m_socket, &QAbstractSocket::readyRead, m_reader,
            &WebExtReader::readyRead);
    connect(m_reader, &WebExtReader::messageReceived, this,
            [&](const QByteArray& data) { emit messageReceived(data); });

    emit connected();
  } else if (m_reader != nullptr) {
    delete m_reader;
    m_reader = nullptr;
    emit disconnected();
  }
}

void WebExtBridge::errorOccurred(QAbstractSocket::SocketError socketError) {
  //qDebug() << "Web extension socket error:" << m_socket.errorString();
}

void WebExtBridge::retryConnection() {
  // Abort and try to reconnect.
  if (m_socket.state() != QAbstractSocket::UnconnectedState) {
    m_socket.abort();
  }
  m_socket.connectToHost(QHostAddress(QHostAddress::LocalHost), m_port);
}

bool WebExtBridge::sendMessage(const QByteArray& message) {
  if (m_socket.state() != QAbstractSocket::ConnectedState) {
    return false;
  }

  quint32 length = message.length();
  m_buffer.append(reinterpret_cast<char*>(&length), sizeof(quint32));
  m_buffer.append(message);
  tryPushData();
  return true;
}

void WebExtBridge::tryPushData() {
  if (m_buffer.isEmpty()) {
    return;
  }
  qint64 len = m_socket.write(m_buffer);
  if (len > 0) {
    m_buffer = m_buffer.sliced(len);
  }
}

int WebExtBridge::run(int argc, char** argv) {
  QCoreApplication app(argc, argv);

  // Prepare to parse the command line arguments.
  QCommandLineParser parser;
  const QString desc = "Web extension bridge to the Mozilla VPN client";
  parser.setApplicationDescription(desc);
  parser.addPositionalArgument("manifest", "Path to the extension manifest");
  parser.addPositionalArgument("ext-id", "The extension idedentifier");

  QCommandLineOption optVersion = parser.addVersionOption();
  QCommandLineOption optHelp = parser.addHelpOption();
  QCommandLineOption optPort(QStringList({"p", "port"}),
                             "Port number of the Mozilla VPN client", "PORT");
  optPort.setDefaultValue(QString::number(VPN_WEBEXTENSION_PORT));
  parser.addOption(optPort);

  // Parse command line arguments.
  parser.parse(app.arguments());
  if (parser.isSet(optHelp)) {
    parser.showHelp();
    return 0;
  }
  if (parser.isSet(optVersion)) {
    parser.showVersion();
    return 0;
  }

  quint16 portNum = VPN_WEBEXTENSION_PORT;
  if (parser.isSet(optPort)) {
    bool okay = false;
    QString vstring = parser.value(optPort);
    ulong value = vstring.toULong(&okay);
    if (!okay || value > UINT16_MAX) {
      qWarning() << "Invalid port specified:" << vstring;
      return 1;
    }
    portNum = value;
  }

  QStringList args = parser.positionalArguments();
  if (args.length() != 2) { 
    qWarning() << "Expected 2 arguments got:" << args.length();
    return 1;
  }
  const QString extId = args.last();
  if (!ALLOW_LISTED_WEBEXTENSIONS.contains(extId)) {
    qWarning() << app.applicationName() << "is not accessible for extension:"
               << extId;
    return 1;
  }

  QFile stdoutFile;
  stdoutFile.open(stdout, QIODeviceBase::WriteOnly);
  WebExtHandler handler(&stdoutFile);
  QObject::connect(&handler, &WebExtHandler::eofReceived, &app,
                   &QCoreApplication::quit);

  // Create a webextension bridge to the VPN client.
  WebExtBridge bridge(portNum);
  QObject::connect(&bridge, &WebExtBridge::messageReceived, &handler,
                   &WebExtHandler::writeMsgStdout);
  QObject::connect(&handler, &WebExtHandler::unhandledMessage, &bridge,
                   [&](const QByteArray msg) {
                     if (!bridge.sendMessage(msg)) {
                       QJsonObject obj({{"error", "vpn-client-down"}});
                       handler.writeJsonStdout(obj);
                     }
                    });
  QObject::connect(&bridge, &WebExtBridge::connected, &handler, [&]() {
    handler.writeJsonStdout(QJsonObject({{"status", "vpn-client-up"}}));
  });
  QObject::connect(&bridge, &WebExtBridge::disconnected, &handler, [&]() {
    handler.writeJsonStdout(QJsonObject({{"error", "vpn-client-down"}}));
  });

  // Run the web extension bridge.
  QObject::connect(qApp, &QCoreApplication::aboutToQuit,
    []() { qInfo() << "Shutting down"; });
  return app.exec();
}
