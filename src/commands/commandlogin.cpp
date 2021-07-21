/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogin.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "localizer.h"
#include "models/devicemodel.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/passwordauth/taskpasswordauth.h"

#ifdef MVPN_WINDOWS
#  include <windows.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

#include <QEventLoop>
#include <QScopeGuard>
#include <QTextStream>

CommandLogin::CommandLogin(QObject* parent)
    : Command(parent, "login", "Starts the authentication flow.") {
  MVPN_COUNT_CTOR(CommandLogin);
}

CommandLogin::~CommandLogin() { MVPN_COUNT_DTOR(CommandLogin); }

int CommandLogin::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option verboseOption("v", "verbose", "Verbose mode.");
  CommandLineParser::Option passwordOption("p", "password",
                                           "Login using e-mail and password");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&verboseOption);
  options.append(&passwordOption);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (!tokens.isEmpty()) {
    return clp.unknownOption(this, appName, tokens[0], options, false);
  }

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  // Authenticate with FxA e-mail and password
  if (passwordOption.m_set)
    return runCommandLineApp([&]() {
      QString email = getInput("Username:");
      QString password = getPassword("Password:");

      MozillaVPN vpn;
      TaskPasswordAuth task(email, password);
      task.run(&vpn);

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, [&] { loop.exit(); });
      loop.exec();
      return 0;
    });

  // Authenticate with a browser session
  return runGuiApp([&] {
    if (SettingsHolder::instance()->hasToken()) {
      QTextStream stream(stdout);
      stream << "User status: already authenticated" << Qt::endl;
      return 1;
    }
    MozillaVPN vpn;
    vpn.authenticate();

    QEventLoop loop;
    QObject::connect(&vpn, &MozillaVPN::stateChanged, [&] {
      if (vpn.state() == MozillaVPN::StatePostAuthentication ||
          vpn.state() == MozillaVPN::StateTelemetryPolicy ||
          vpn.state() == MozillaVPN::StateMain) {
        loop.exit();
      }
    });
    loop.exec();

    if (!vpn.deviceModel()->hasCurrentDevice(vpn.keys())) {
      QTextStream stream(stdout);
      stream << "Device limit reached" << Qt::endl;
      return 1;
    }

    return 0;
  });
}

// static
QString CommandLogin::getInput(const QString& prompt) {
  QTextStream stream(stdout);
  stream << prompt << " ";
  stream.flush();

  return QTextStream(stdin).readLine();
}

// static
QString CommandLogin::getPassword(const QString& prompt) {
  // Disable the console echo while typing the password
#ifdef MVPN_WINDOWS
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);
  SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
  auto guard = qScopeGuard([&] { SetConsoleMode(hStdin, mode); });
#else
  struct termios tty;
  struct termios noecho;
  tcgetattr(STDIN_FILENO, &tty);
  memcpy(&noecho, &tty, sizeof(struct termios));
  noecho.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &noecho);
  auto guard = qScopeGuard([&] { tcsetattr(STDIN_FILENO, TCSANOW, &tty); });
#endif

  QString result = getInput(prompt);
  QTextStream stream(stdout);
  stream << Qt::endl;
  return result;
}

static Command::RegistrationProxy<CommandLogin> s_commandLogin;
