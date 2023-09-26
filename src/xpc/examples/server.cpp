#include <stdio.h>

#include <QCoreApplication>
#include <QObject>
#include <QScopedPointer>

#include "../xpcservice.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  XPCService service;

  // Just echo back.
  QObject::connect(&service, &XPCService::messageReceived,
                   [&](const QString msg) { service.send(msg); });
  service.start();

  return app.exec();
}
