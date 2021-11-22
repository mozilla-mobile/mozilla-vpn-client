/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogout.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "localizer.h"
#include "models/device.h"
#include "mozillavpn.h"
#include "tasks/removedevice/taskremovedevice.h"

#include <QEventLoop>
#include <QTextStream>

CommandLogout::CommandLogout(QObject* parent)
    : Command(parent, "logout", "Logout the current user.") {
  MVPN_COUNT_CTOR(CommandLogout);
}

CommandLogout::~CommandLogout() { MVPN_COUNT_DTOR(CommandLogout); }

int CommandLogout::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    if (tokens.length() > 1) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    if (!userAuthenticated()) {
      return 1;
    }

    if (!loadModels()) {
      QTextStream stream(stdout);
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    const Device* currentDevice =
        MozillaVPN::instance()->deviceModel()->currentDevice(
            MozillaVPN::instance()->keys());
    if (currentDevice) {
      TaskRemoveDevice task(currentDevice->publicKey());
      task.run();

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, [&] { loop.exit(); });
      loop.exec();
    }
    MozillaVPN::instance()->reset(false);

    return 0;
  });
}

static Command::RegistrationProxy<CommandLogout> s_commandLogout;
