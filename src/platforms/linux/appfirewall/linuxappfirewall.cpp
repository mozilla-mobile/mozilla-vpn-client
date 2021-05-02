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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

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
      QString busPath;
      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      if (env.contains("DBUS_SESSION_BUS_ADDRESS")) {
        busPath = env.value("DBUS_SESSION_BUS_ADDRESS");
      }
      /* If we were launched via sudo, connect to the real user's session. */
      else if (env.contains("SUDO_UID")) {
        busPath = "unix:path=/run/user/" + env.value("SUDO_UID") + "/bus";
      } else {
        logger.log() << "Could not locate session D-Bus, consider setting "
                        "DBUS_SESSION_BUS_ADDRESS";
        return 1;
      }

      /* Assume the effective UID of the bus before connecting, since we are
       * starting as root but the D-Bus security policy will prohibit access
       * from different UIDs
       */
      struct stat st;
      uid_t realuid = getuid();
      int err = stat(busPath.section('=', 1, 1).toLocal8Bit().data(), &st);
      if (err < 0) {
        logger.log() << "Unable to stat() D-Bus session path:"
                     << strerror(errno);
        return 1;
      }
      if (realuid != st.st_uid) {
        seteuid(st.st_uid);
      }
      QDBusConnection connection =
          QDBusConnection::connectToBus(busPath, "usersession");
      if (realuid != st.st_uid) {
        seteuid(realuid);
      }
      if (!connection.isConnected()) {
        logger.log() << "Unable to connect to D-Bus session"
                     << connection.lastError().message();
        return 1;
      }

      AppTracker* apptracker = new AppTracker(connection, qApp);
      FirewallService* firewall = new FirewallService(qApp);

      QObject::connect(apptracker, &AppTracker::appLaunched, firewall,
                       &FirewallService::trackApp);

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
