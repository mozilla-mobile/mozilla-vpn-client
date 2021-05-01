/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QApplication>

#include "apptracker.h"
#include "command.h"
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

  void logFork(const QString& name, int parent, int child) {
    logger.log() << "fork:" << name << "PID:" << parent << "->" << child;
  }
  void logExit(const QString& name, int pid) {
    logger.log() << "exit:" << name << "PID:" << pid;
  }
  void logLaunch(const QString& name, int pid) {
    logger.log() << "launch:" << name << "PID:" << pid;
  }
  void logTerm(const QString& name, int rootpid) {
    logger.log() << "terminate:" << name << "PID:" << rootpid;
  }

  int run(QStringList& tokens) override {
    Q_ASSERT(!tokens.isEmpty());

    return runCommandLineApp([&]() {
      PidTracker* pidtracker = new PidTracker(qApp);
      AppTracker* apptracker = new AppTracker(qApp);

      QObject::connect(pidtracker, &PidTracker::pidForked, this,
                       &CommandLinuxAppFirewall::logFork);
      QObject::connect(pidtracker, &PidTracker::pidExited, this,
                       &CommandLinuxAppFirewall::logExit);
      QObject::connect(pidtracker, &PidTracker::terminated, this,
                       &CommandLinuxAppFirewall::logTerm);
      QObject::connect(apptracker, &AppTracker::appLaunched, this,
                       &CommandLinuxAppFirewall::logLaunch);
      QObject::connect(apptracker, &AppTracker::appLaunched, pidtracker,
                       &PidTracker::track);

      SignalHandler sh;
      QObject::connect(&sh, &SignalHandler::quitRequested,
                       [&]() { qApp->quit(); });

      pidtracker->initialize();

      logger.log() << "Ready!";
      return qApp->exec();
    });
  }
};

static Command::RegistrationProxy<CommandLinuxAppFirewall>
    s_commandLinuxAppFirewall;
