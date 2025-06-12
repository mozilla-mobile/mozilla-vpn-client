/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>

#include "command.h"
#include "commandlineparser.h"
#include "dbus_adaptor.h"
#include "dbusservice.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "signalhandler.h"

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
    LogHandler::setLogfile("/var/log/mozillavpn.log");

    QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());
    DBusService* dbus = new DBusService(&app);
    new DbusAdaptor(dbus);

    QDBusConnection connection = QDBusConnection::systemBus();
    logger.debug() << "Connecting to DBus...";

    if (!connection.registerService("org.mozilla.vpn.dbus") ||
        !connection.registerObject("/", dbus)) {
      logger.error() << "Connection failed - name:"
                     << connection.lastError().name()
                     << "message:" << connection.lastError().message();
      return 1;
    }

    // Signal handling for a proper shutdown.
    SignalHandler sh;
    QObject::connect(&sh, &SignalHandler::quitRequested, &sh, [&]() {
      dbus->deactivate();
      app.quit();
    });

    logger.debug() << "Ready!";
    return app.exec();
  }
};

static Command::RegistrationProxy<CommandLinuxDaemon> s_commandLinuxDaemon;
