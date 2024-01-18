/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogin.h"

#include "authenticationinapp/authenticationinapp.h"
#include "authenticationlistener.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "localizer.h"
#include "models/devicemodel.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/authenticate/taskauthenticate.h"

#ifdef _WIN32  // Avoid using MZ_WINDOWS here as it conflicts with MZ_DUMMY on
               // Windows
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

  return runGuiApp([&] {
    if (SettingsHolder::instance()->hasToken()) {
      QTextStream stream(stdout);
      stream << "User status: already authenticated" << Qt::endl;
      return 1;
    }

    MozillaVPN vpn;

    if (!passwordOption.m_set) {
      vpn.authenticateWithType(AuthenticationListener::AuthenticationInBrowser);
    } else {
      vpn.authenticateWithType(AuthenticationListener::AuthenticationInApp);
    }

    QEventLoop loop;

    if (passwordOption.m_set) {
      AuthenticationInApp* aia = AuthenticationInApp::instance();

      QObject::connect(aia, &AuthenticationInApp::stateChanged, aia, [&] {
        switch (AuthenticationInApp::instance()->state()) {
          case AuthenticationInApp::StateInitializing:
            break;

          case AuthenticationInApp::StateStart: {
            QString email = getInput("Username:");
            AuthenticationInApp::instance()->checkAccount(email);
          } break;

          case AuthenticationInApp::StateCheckingAccount:
            break;

          case AuthenticationInApp::StateSignIn: {
            QString password = getPassword("Password:");
            AuthenticationInApp::instance()->setPassword(password);
            AuthenticationInApp::instance()->signIn();
          } break;

          case AuthenticationInApp::StateSigningIn:
            break;

          case AuthenticationInApp::StateSignUp: {
            QTextStream stream(stdout);
            stream << "Sign up is not supported in CLI mode." << Qt::endl;
            loop.exit();
          } break;

          case AuthenticationInApp::StateSigningUp:
            break;

          case AuthenticationInApp::StateUnblockCodeNeeded: {
            QString code = getInput("Check your email. Unblock code:");
            AuthenticationInApp::instance()->verifyUnblockCode(code);
          } break;

          case AuthenticationInApp::StateVerifyingUnblockCode:
            break;

          case AuthenticationInApp::StateVerificationSessionByEmailNeeded: {
            AuthenticationInApp::instance()
                ->resendVerificationSessionCodeEmail();
            QString code =
                getInput("Session verification by email needed. Code:");
            AuthenticationInApp::instance()->verifySessionEmailCode(code);
          } break;

          case AuthenticationInApp::StateVerifyingSessionEmailCode:
            break;

          case AuthenticationInApp::StateVerificationSessionByTotpNeeded: {
            QString code =
                getInput("Session verification by TOTP needed. Code:");
            AuthenticationInApp::instance()->verifySessionTotpCode(code);
          } break;

          case AuthenticationInApp::StateVerifyingSessionTotpCode:
            break;

          case AuthenticationInApp::StateIsStubAccount: {
            QTextStream stream(stdout);
            stream << "This account is a stub account, i.e. it does not yet "
                      "have a password. Please check your email, and follow "
                      "the verification link sent by Mozilla Accounts to set a "
                      "password, and try again."
                   << Qt::endl;
            loop.exit();
          } break;

          case AuthenticationInApp::StateIsSsoAccount: {
            QTextStream stream(stdout);
            stream << "This account is linked to a third-party authentication "
                      "provider (i.e. Google or Apple), but does not have a "
                      "password associated with it. Mozilla VPN requires all "
                      "accounts to be associated with a password before they "
                      "can be used to authenticate with the client. Please "
                      "visit https://accounts.firefox.com/settings/password to "
                      "set a password, then try to log in again."
                   << Qt::endl;
            loop.exit();
          } break;

          case AuthenticationInApp::StateAccountDeletionRequest:
            Q_ASSERT(false);
            break;

          case AuthenticationInApp::StateDeletingAccount:
            Q_ASSERT(false);
            break;

          case AuthenticationInApp::StateFallbackInBrowser: {
            QTextStream stream(stdout);
            stream << "Unable to continue with the flow. Please, continue the "
                      "login in browser.";
            loop.exit();
          } break;
        }
      });

      QObject::connect(
          aia, &AuthenticationInApp::errorOccurred,
          [&](AuthenticationInApp::ErrorType error) {
            QTextStream stream(stdout);
            switch (error) {
              case AuthenticationInApp::ErrorAccountAlreadyExists:
                [[fallthrough]];
              case AuthenticationInApp::ErrorUnknownAccount:
                stream << "Unknown account" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorIncorrectPassword:
                stream << "Incorrect password!" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorInvalidUnblockCode:
                stream << "Invalid unblock code!" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorInvalidEmailAddress:
                stream << "Invalid email address" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorInvalidOrExpiredVerificationCode:
                stream << "Invalid or expired verification code!" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorEmailTypeNotSupported:
                stream << "Email type not supported" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorEmailCanNotBeUsedToLogin:
                stream << "This email can not be used to login" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorFailedToSendEmail:
                stream << "Failed to send email" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorTooManyRequests:
                stream << "Too many requests. Slow down, please." << Qt::endl;
                break;
              case AuthenticationInApp::ErrorServerUnavailable:
                stream << "The server is down" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorInvalidTotpCode:
                stream << "Invalid TOTP code" << Qt::endl;
                break;
              case AuthenticationInApp::ErrorConnectionTimeout:
                stream << "Request Timed Out" << Qt::endl;
                break;
            }
          });
    }

    QObject::connect(&vpn, &App::stateChanged, &vpn, [&] {
      if (vpn.state() == App::StatePostAuthentication ||
          vpn.state() == App::StateTelemetryPolicy ||
          vpn.state() == App::StateMain) {
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
