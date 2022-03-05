/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/authenticationinapp/authenticationinapp.h"
#include "../../src/constants.h"
#include "../../src/settingsholder.h"
#include "../../src/simplenetworkmanager.h"
#include "../../src/tasks/authenticate/taskauthenticate.h"
#include "../../tests/auth/utils.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

constexpr const char* PASSWORD = "12345678";

int create(bool jsonOutput) {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);
  Q_ASSERT(aia->state() == AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  QEventLoop loop;
  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() == AuthenticationInApp::StateStart);

  QString emailAccount("vpn.test.");
  emailAccount.append(QString::number(QDateTime::currentSecsSinceEpoch()));
  QString emailAddress(emailAccount);
  emailAddress.append("@restmail.net");

  aia->checkAccount(emailAddress);
  Q_ASSERT(aia->state() == AuthenticationInApp::StateCheckingAccount);

  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    Q_ASSERT(aia->state() != AuthenticationInApp::StateSignIn);
    if (aia->state() == AuthenticationInApp::StateSignUp) {
      loop.exit();
    }
  });
  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() == AuthenticationInApp::StateSignUp);

  // Password
  aia->setPassword(PASSWORD);

  // Sign-up
  aia->signUp();
  Q_ASSERT(aia->state() == AuthenticationInApp::StateSigningUp);

  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() ==
        AuthenticationInApp::StateVerificationSessionByEmailNeeded) {
      loop.exit();
    }
  });
  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() ==
           AuthenticationInApp::StateVerificationSessionByEmailNeeded);

  // Email verification - with valid code
  QString code = TestUtils::fetchSessionCode(emailAccount);
  Q_ASSERT(!code.isEmpty());
  aia->verifySessionEmailCode(code);
  Q_ASSERT(aia->state() == AuthenticationInApp::StateVerifyingSessionEmailCode);

  QUrl finalUrl;
  QObject::connect(aia, &AuthenticationInApp::unitTestFinalUrl,
                   [&](const QUrl& url) { finalUrl = url; });
  QObject::connect(&task, &Task::completed, [&]() { loop.exit(); });

  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  // The account is not active yet. So, let's check the final URL.
  Q_ASSERT(
      (finalUrl.host() == "stage-vpn.guardian.nonprod.cloudops.mozgcp.net" &&
       finalUrl.path() == "/vpn/client/login/success") ||
      (finalUrl.host() == "www-dev.allizom.org" &&
       finalUrl.path() == "/en-US/products/vpn/"));

  QTextStream out(stdout);

  if (!jsonOutput) {
    out << "Account: " << emailAccount << Qt::endl;
    out << "Email address: " << emailAddress << Qt::endl;
    out << "Password: " << PASSWORD << Qt::endl;
  } else {
    QJsonObject obj;
    obj["account"] = emailAccount;
    obj["emailAddress"] = emailAddress;
    obj["password"] = PASSWORD;
    out << QJsonDocument(obj).toJson() << Qt::endl;
  }
  return 0;
}

int destroy(const QString& emailAccount, const QString& password) {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(!!aia);
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() == AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  QEventLoop loop;
  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() == AuthenticationInApp::StateStart);

  QString emailAddress(emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia->checkAccount(emailAddress);
  Q_ASSERT(aia->state() == AuthenticationInApp::StateCheckingAccount);

  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateSignIn) {
      loop.exit();
    }
  });

  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  Q_ASSERT(aia->state() == AuthenticationInApp::StateSignIn);

  // Password
  aia->setPassword(password);

  // Let's delete the account the end of the flow.
  aia->enableAccountDeletion();

  // Sign-in
  aia->signIn();
  Q_ASSERT(aia->state() == AuthenticationInApp::StateSigningIn);

  QObject::connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateUnblockCodeNeeded) {
      AuthenticationInApp* aia = AuthenticationInApp::instance();

      // We do not receive the email each time.
      aia->resendUnblockCodeEmail();

      QString code = TestUtils::fetchUnblockCode(emailAccount);
      Q_ASSERT(!code.isEmpty());
      aia->verifyUnblockCode(code);
      Q_ASSERT(aia->state() == AuthenticationInApp::StateVerifyingUnblockCode);
    }
  });

  QUrl finalUrl;
  QObject::connect(aia, &AuthenticationInApp::unitTestFinalUrl,
                   [&](const QUrl& url) { finalUrl = url; });
  QObject::connect(&task, &Task::completed, [&]() { loop.exit(); });

  loop.exec();
  QObject::disconnect(aia, nullptr, nullptr, nullptr);

  // The account is not active yet. So, let's check the final URL.
  Q_ASSERT(
      (finalUrl.host() == "stage-vpn.guardian.nonprod.cloudops.mozgcp.net" &&
       finalUrl.path() == "/vpn/client/login/success") ||
      (finalUrl.host() == "www-dev.allizom.org" &&
       finalUrl.path() == "/en-US/products/vpn/"));
  return 0;
}

int main(int argc, char* argv[]) {
  SettingsHolder settingsHolder;
  Constants::setStaging();

  settingsHolder.setDevModeFeatureFlags(
      QStringList{"inAppAuthentication", "inAppAccountCreate"});

  QCoreApplication app(argc, argv);

  QCoreApplication::setApplicationName("Mozilla VPN - AuthHelper");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  QCommandLineParser parser;
  parser.setApplicationDescription("Mozilla VPN - AuthHelper");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("operation",
                               "Operation: create, delete [account password]");

  QCommandLineOption showJsonOutput("j", "Show JSON output");
  parser.addOption(showJsonOutput);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.isEmpty()) {
    parser.showHelp(1);
  }

  SimpleNetworkManager snm;
  FeatureList::instance()->initialize();

  if (args[0] == "create") {
    if (args.length() > 1) {
      parser.showHelp(1);
    }

    return create(parser.isSet(showJsonOutput));
  }

  if (args[0] == "destroy") {
    if (args.length() != 3) {
      parser.showHelp(1);
    }

    return destroy(args[1], args[2]);
  }

  parser.showHelp(1);
  return 1;
}
