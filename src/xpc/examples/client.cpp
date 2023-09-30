#include <stdio.h>

#include <QCoreApplication>
#include <QObject>
#include <QScopedPointer>
#include <QThread>
#include <QtLogging>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../xpcclient.h"

class Reciever : public QObject {
  Q_OBJECT
 public:
  Reciever(QObject* aParent) : QObject(aParent){};
  ~Reciever(){};

  Q_SLOT void print(QString msg) {
    qWarning() << msg;
    std::cout << "Got: " << msg.toLocal8Bit().constData();
  };
};
#include "client.moc"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  // Make sure this one lives on the app thread.
  Reciever r(&app);

  QThread t;
  XPCClient client;
  client.moveToThread(&t);
  auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
  qWarning() << "[App] - Thread " << qUtf8Printable(QString::number(h));

  // Print
  QObject::connect(&client, &XPCClient::messageReceived, &r, &Reciever::print,
                   Qt::QueuedConnection);
  QObject::connect(&client, &XPCClient::onConnectionError, &r, &Reciever::print,
                   Qt::QueuedConnection);
  t.start();
  client.connectService("org.mozilla.firefox.vpn.daemon.test");
  qWarning("[App] - RUN event loop ");
  return app.exec();
}
