/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogout.h"

#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "leakdetector.h"
#include "localizer.h"
#include "models/device.h"
#include "models/devicemodel.h"
#include "mozillavpn.h"
#include "tasks/removedevice/taskremovedevice.h"

CommandLogout::CommandLogout(QObject* parent)
    : Command(parent, "logout", "Logout the current user.") {
  MZ_COUNT_CTOR(CommandLogout);
}

CommandLogout::~CommandLogout() { MZ_COUNT_DTOR(CommandLogout); }

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

    MozillaVPN vpn;

    if (!loadModels()) {
      QTextStream stream(stdout);
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    const Device* currentDevice = vpn.deviceModel()->currentDevice(vpn.keys());
    if (currentDevice) {
      TaskRemoveDevice task(currentDevice->publicKey());
      task.run();

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, &task, [&] { loop.exit(); });
      loop.exec();
    }
    vpn.reset(false);

    return 0;
  });
}

static Command::RegistrationProxy<CommandLogout> s_commandLogout;
