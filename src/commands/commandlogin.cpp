/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogin.h"

#include "authenticationlistener.h"
#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "daemon/mock/mockdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/devicemodel.h"
#include "models/serverdata.h"
#include "mozillavpn.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "taskscheduler.h"

#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
#  include "eventlistener.h"
#endif

#ifdef _WIN32  // Avoid using MZ_WINDOWS here as it conflicts with MZ_DUMMY on
               // Windows
#  include <windows.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

#include <QCoreApplication>
#include <QEventLoop>
#include <QScopeGuard>
#include <QTextStream>

CommandLogin::CommandLogin(QObject* parent)
    : Command(parent, "login", "Starts the authentication flow.") {
  MZ_COUNT_CTOR(CommandLogin);
}

CommandLogin::~CommandLogin() { MZ_COUNT_DTOR(CommandLogin); }

int CommandLogin::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option verboseOption("v", "verbose", "Verbose mode.");
  CommandLineParser::Option passwordOption("p", "password",
                                           "Login using e-mail and password");
  CommandLineParser::Option headlessOption("d", "headless",
                                           "Login in headless mode.");
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&verboseOption);
  options.append(&passwordOption);
  options.append(&headlessOption);
  options.append(&testingOption);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  if (testingOption.m_set) {
    QCoreApplication::setOrganizationName("Mozilla Testing");

    LogHandler::instance()->setStderr(true);
  }

  // If there is another instance, the execution terminates here.
#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
  if (EventListener::checkForInstances()) {
    QTextStream stream(stderr);
    stream << "Existing instance found" << Qt::endl;

    // If we are given URL parameters, send them to the UI socket and exit.
    for (const QString& value : tokens) {
      QUrl url(value);
      if (!url.isValid() || (url.scheme() != Constants::DEEP_LINK_SCHEME)) {
        stream << "Invalid link:" << value << Qt::endl;
      } else {
        stream << "Sending link" << Qt::endl;
        EventListener::sendDeepLink(url);
      }
    }

    return 0;
  }
  // This class receives communications from other instances.
  std::unique_ptr<EventListener> eventListener;
#endif

  return MozillaVPN::runCommandLineApp([&] {
    MozillaVPN vpn;
    if (testingOption.m_set) {
      Constants::setStaging();
    }
    if (vpn.hasToken()) {
      QTextStream stream(stdout);
      stream << "User status: already authenticated" << Qt::endl;
      return 1;
    }

    QTextStream stream(stdout);

    if (passwordOption.m_set) {
      stream << "Password-based authentication is not supported anymore"
             << Qt::endl;
      return 1;
    }
  
    vpn.serverData()->initialize();
#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
    eventListener.reset(new EventListener{});
#endif
    if (headlessOption.m_set) {
      vpn.authenticateWithType(
          AuthenticationListener::AuthenticationInBrowserHeadless);
      QObject::connect(&vpn, &MozillaVPN::authenticationStarted, this, [&] {
        QString code = getInput("Enter the code:");
        auto task =
            qobject_cast<TaskAuthenticate*>(TaskScheduler::runningTask());
        if (task) {
          task->authenticatePkceSuccess(code);
        }
      });
    } else {
      vpn.authenticateWithType(AuthenticationListener::AuthenticationInBrowser);
    }

    QEventLoop loop;

    QObject::connect(&vpn, &MozillaVPN::stateChanged, &vpn, [&] {
      if (vpn.state() == MozillaVPN::StateMain ||
          vpn.state() == MozillaVPN::StateOnboarding) {
        loop.exit();
      }
      if (ErrorHandler::instance()->alert() ==
          ErrorHandler::AuthenticationFailedAlert) {
        loop.exit();
      }
    });

    loop.exec();

    if (ErrorHandler::instance()->alert() ==
        ErrorHandler::AuthenticationFailedAlert) {
      QTextStream stream(stdout);
      stream << "Authentication failed" << Qt::endl;
      return 1;
    }
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
#ifdef _WIN32  // Avoid using MZ_WINDOWS here as it conflicts with MZ_DUMMY on
               // Windows
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
