/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/logger.h"
#include "../../src/loghandler.h"
#include "../../src/signalhandler.h"
#include "dbus.h"
#include "dbus_adaptor.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_LINUX, "main");
}

int main(int argc, char* argv[]) {
  qInstallMessageHandler(LogHandler::messageQTHandler);

  QCoreApplication app(argc, argv);

  DBus* dbus = new DBus(&app);
  DbusAdaptor* adaptor = new DbusAdaptor(dbus);
  dbus->setAdaptor(adaptor);

  QDBusConnection connection = QDBusConnection::systemBus();
  logger.log() << "Connecting to DBus...";

  if (!connection.registerService("org.mozilla.vpn.dbus") ||
      !connection.registerObject("/", dbus)) {
    logger.log() << "Connection failed - name:" << connection.lastError().name()
                 << "message:" << connection.lastError().message();
    app.exit(1);
    return 1;
  }

  if (!dbus->checkInterface()) {
    app.exit(1);
    return 1;
  }

  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested, [&]() {
    dbus->deactivate();
    app.quit();
  });

  logger.log() << "Ready!";
  return app.exec();
}
