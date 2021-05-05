/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QApplication>

#include "apptracker.h"
#include "command.h"
#include "firewallservice.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "pidtracker.h"
#include "signalhandler.h"

namespace {
Logger logger(LOG_LINUX, "main");
}

class CommandLinuxAppFirewall final : public Command {
 public:
  explicit CommandLinuxAppFirewall(QObject* parent)
      : Command(parent, "linuxappfirewall",
                "Starts the linux application firewall daemon") {
    MVPN_COUNT_CTOR(CommandLinuxAppFirewall);
  }

  ~CommandLinuxAppFirewall() { MVPN_COUNT_DTOR(CommandLinuxAppFirewall); }

  int run(QStringList& tokens) override {
    Q_ASSERT(!tokens.isEmpty());

    return runCommandLineApp([&]() {
      FirewallService* firewall = new FirewallService(qApp);

      QDBusConnection connection = QDBusConnection::systemBus();
      logger.log() << "Connecting to DBus...";

      if (!connection.registerService("org.mozilla.vpn.firewall") ||
          !connection.registerObject("/org/mozilla/vpn/firewall", firewall)) {
        logger.log() << "Connection failed - name:"
                     << connection.lastError().name()
                     << "message:" << connection.lastError().message();
        return 1;
      }

      SignalHandler sh;
      QObject::connect(&sh, &SignalHandler::quitRequested,
                       [&]() { qApp->quit(); });

      firewall->initialize();

      logger.log() << "Ready!";
      return qApp->exec();
    });
  }
};

static Command::RegistrationProxy<CommandLinuxAppFirewall>
    s_commandLinuxAppFirewall;
