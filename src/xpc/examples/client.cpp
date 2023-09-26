#include <stdio.h>

#include <QCoreApplication>
#include <QObject>
#include <QScopedPointer>

#include "../xpcclient.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  XPCClient client;

  // Print
  QObject::connect(
      &client, &XPCClient::messageReceived,
      [&](const QString msg) { std::cout << msg.toLocal8Bit().constData(); });
  client.start();

  return app.exec();
}
