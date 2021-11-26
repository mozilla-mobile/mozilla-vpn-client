/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsignupandin.h"
#include "../../src/authenticationinapp/authenticationinapp.h"
#include "../../src/networkrequest.h"
#include "../../src/tasks/authenticate/taskauthenticate.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTest>

#include <liboath/oath.h>

constexpr const char* PASSWORD = "12345678";

class EventLoop final : public QEventLoop {
 public:
  void exec() {
    QTimer timer;
    connect(&timer, &QTimer::timeout, [&]() {
      qDebug() << "TIMEOUT!";
      exit();
    });
    timer.setSingleShot(true);
    timer.start(60000 /* 60 seconds */);

    QEventLoop::exec();
  }
};

TestSignUpAndIn::TestSignUpAndIn(const QString& pattern, bool totpCreation)
    : m_totpCreation(totpCreation) {
  QString emailAccount(pattern);
  emailAccount.append(QString::number(QDateTime::currentSecsSinceEpoch()));
  m_emailAccount = emailAccount;
  qDebug() << "Pattern:" << m_emailAccount;
}

void TestSignUpAndIn::signUp() {
  auto& aia = AuthenticationInApp::instance();
  disconnect(&aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia.state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  EventLoop loop;
  connect(&aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia.state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia.state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia.checkAccount(emailAddress);
  connect(&aia, &AuthenticationInApp::stateChanged, [&]() {
    QVERIFY(aia.state() != AuthenticationInApp::StateSignIn);
    if (aia.state() == AuthenticationInApp::StateSignUp) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia.state(), AuthenticationInApp::StateSignUp);

  // Password
  aia.setPassword(PASSWORD);

  if (m_totpCreation) {
    aia.enableTotpCreation();
  }

  // Sign-up
  aia.signUp();

  connect(&aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia.state() ==
        AuthenticationInApp::StateVerificationSessionByEmailNeeded) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia.state(),
           AuthenticationInApp::StateVerificationSessionByEmailNeeded);

  // Email verification
  QString code = fetchSessionCode();
  QVERIFY(!code.isEmpty());
  aia.verifySessionEmailCode(code);

  QUrl finalUrl;
  connect(&aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });

  if (m_totpCreation) {
    waitForTotpCodes();
  }

  loop.exec();

  // The account is not active yet. So, let's check the final URL.
  QVERIFY(
      (finalUrl.host() == "stage-vpn.guardian.nonprod.cloudops.mozgcp.net" &&
       finalUrl.path() == "/vpn/client/login/success") ||
      (finalUrl.host() == "www-dev.allizom.org" &&
       finalUrl.path() == "/en-US/products/vpn/"));
  qDebug() << finalUrl.path();
}

void TestSignUpAndIn::signIn() {
  auto& aia = AuthenticationInApp::instance();
  disconnect(&aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia.state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  EventLoop loop;
  connect(&aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia.state() == AuthenticationInApp::StateUnblockCodeNeeded) {
      // We do not receive the email each time.
      AuthenticationInApp::instance().resendUnblockCodeEmail();

      QString code = fetchUnblockCode();
      QVERIFY(!code.isEmpty());
      aia.setUnblockCodeAndContinue(code);
      return;
    }

    if (aia.state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia.state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia.checkAccount(emailAddress);
  connect(&aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia.state() == AuthenticationInApp::StateSignIn) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia.state(), AuthenticationInApp::StateSignIn);

  // Password
  aia.setPassword(PASSWORD);

  // Sign-in
  aia.signIn();

  if (m_totpCreation) {
    waitForTotpCodes();
  }

  QUrl finalUrl;
  connect(&aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });

  loop.exec();

  // The account is not active yet. So, let's check the final URL.
  QVERIFY(
      (finalUrl.host() == "stage-vpn.guardian.nonprod.cloudops.mozgcp.net" &&
       finalUrl.path() == "/vpn/client/login/success") ||
      (finalUrl.host() == "www-dev.allizom.org" &&
       finalUrl.path() == "/en-US/products/vpn/"));
  qDebug() << finalUrl.path();
}

QString TestSignUpAndIn::fetchSessionCode() {
  return fetchCode("x-verify-short-code");
}

QString TestSignUpAndIn::fetchUnblockCode() {
  return fetchCode("x-unblock-code");
}

void TestSignUpAndIn::waitForTotpCodes() {
  qDebug() << "Adding the callback for the totp code creation";

  auto& aia = AuthenticationInApp::instance();

  connect(&aia, &AuthenticationInApp::errorOccurred,
          [this](AuthenticationInApp::ErrorType error) {
            if (error == AuthenticationInApp::ErrorInvalidTotpCode) {
              qDebug() << "Invalid code. Let's send the right one";

              QCOMPARE(oath_init(), OATH_OK);

              char otp[/* length + 1 */ 7] = {};
              QCOMPARE(oath_totp_generate(m_totpSecret.data(),
                                          m_totpSecret.length(), time(nullptr),
                                          OATH_TOTP_DEFAULT_TIME_STEP_SIZE,
                                          OATH_TOTP_DEFAULT_START_TIME, 6, otp),
                       OATH_OK);

              qDebug() << "Code:" << otp;
              auto& aia = AuthenticationInApp::instance();
              aia.verifySessionTotpCode(otp);
            }
          });

  connect(&aia, &AuthenticationInApp::unitTestTotpCodeCreated,
          [this](const QByteArray& data) {
            qDebug() << "Codes received";
            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();
            QByteArray totpSecret = obj["secret"].toString().toLocal8Bit();

            char* secret = nullptr;
            size_t secretLength = 0;
            QCOMPARE(oath_base32_decode(totpSecret.data(), totpSecret.length(),
                                        &secret, &secretLength),
                     OATH_OK);

            m_totpSecret = QByteArray(secret, secretLength);
            QVERIFY(!m_totpSecret.isEmpty());
          });

  connect(&aia, &AuthenticationInApp::stateChanged, []() {
    auto& aia = AuthenticationInApp::instance();
    if (aia.state() ==
        AuthenticationInApp::StateVerificationSessionByTotpNeeded) {
      qDebug() << "Code required. Let's write a wrong code first.";
      aia.verifySessionTotpCode("123456");
    }
  });
}

QString TestSignUpAndIn::fetchCode(const QString& code) {
  while (true) {
    QString url = "http://restmail.net/mail/";
    url.append(m_emailAccount);

    NetworkRequest* nr = NetworkRequest::createForGetUrl(this, url);

    QByteArray jsonData;
    EventLoop loop;
    connect(nr, &NetworkRequest::requestFailed,
            [&](QNetworkReply::NetworkError, const QByteArray&) {
              qDebug() << "Failed to fetch the restmail.net content";
              loop.exit();
            });
    connect(nr, &NetworkRequest::requestCompleted, [&](const QByteArray& data) {
      jsonData = data;
      loop.exit();
    });
    loop.exec();

    QJsonDocument doc(QJsonDocument::fromJson(jsonData));
    QJsonArray array = doc.array();
    if (!array.isEmpty()) {
      QJsonObject obj = array.last().toObject();
      QJsonObject headers = obj["headers"].toObject();
      if (headers.contains(code)) {
        return headers[code].toString();
      }
    }

    qDebug() << "Email not received yet";

    QTimer timer;
    connect(&timer, &QTimer::timeout, [&]() { loop.exit(); });
    timer.setSingleShot(true);
    timer.start(2000 /* 2 seconds */);
    loop.exec();
  }
}
