/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commanddeeplink.h"

#include <QFileInfo>
#include <QLocalSocket>

#include "command.h"
#include "constants.h"
#include "leakdetector.h"

CommandDeepLink::CommandDeepLink(QObject* parent)
    : Command(parent, "deeplink", "Pass a deep link to the UI instance") {
  MZ_COUNT_CTOR(CommandDeepLink);
}

CommandDeepLink::~CommandDeepLink() { MZ_COUNT_DTOR(CommandDeepLink); }

int CommandDeepLink::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    QTextStream stream(stdout);
    if (tokens.length() != 2) {
      stream << "usage: " << tokens[0] << " <url>" << Qt::endl;
      return 1;
    }

#ifndef MZ_WINDOWS
    if (!QFileInfo::exists(Constants::UI_PIPE)) {
      stream << "UI socket does not exist, is the VPN client running?";
      return 1;
    }
#endif

    QLocalSocket socket;
    socket.connectToServer(Constants::UI_PIPE);
    if (!socket.waitForConnected(1000)) {
      stream << "Connection failed: " << socket.errorString();
      return 1;
    }

    QString message = QString("link %1\n").arg(tokens[1]);
    socket.write(message.toUtf8());
    socket.flush();
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState) {
      socket.waitForDisconnected(1000);
    }

    return 0;
  });
}

static Command::RegistrationProxy<CommandDeepLink> s_commandDeepLink;
