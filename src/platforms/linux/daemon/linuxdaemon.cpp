/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command.h"
#include "dbus_adaptor.h"
#include "dbusservice.h"
#include "logging/logger.h"
#include "logging/loghandler.h"
#include "signalhandler.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("main");
}

class CommandLinuxDaemon final : public Command {
 public:
  explicit CommandLinuxDaemon(QObject* parent)
      : Command(parent, "linuxdaemon", "Starts the linux daemon") {
    MZ_COUNT_CTOR(CommandLinuxDaemon);
  }

  ~CommandLinuxDaemon() { MZ_COUNT_DTOR(CommandLinuxDaemon); }

  int run(QStringList& tokens) override {
    Q_ASSERT(!tokens.isEmpty());
    LogHandler::setLocation("/var/log");

    return runCommandLineApp([&]() {
      DBusService* dbus = new DBusService(qApp);
      DbusAdaptor* adaptor = new DbusAdaptor(dbus);
      dbus->setAdaptor(adaptor);

      QDBusConnection connection = QDBusConnection::systemBus();
      logger.debug() << "Connecting to DBus...";

      if (!connection.registerService("org.mozilla.vpn.dbus") ||
          !connection.registerObject("/", dbus)) {
        logger.error() << "Connection failed - name:"
                       << connection.lastError().name()
                       << "message:" << connection.lastError().message();
        return 1;
      }

      SignalHandler sh;
      QObject::connect(&sh, &SignalHandler::quitRequested, &sh, [&]() {
        dbus->deactivate();
        qApp->quit();
      });

      logger.debug() << "Ready!";
      return qApp->exec();
    });
  }
};

static Command::RegistrationProxy<CommandLinuxDaemon> s_commandLinuxDaemon;
