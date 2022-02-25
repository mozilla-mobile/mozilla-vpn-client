/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsignupandin.h"
#include "../../src/authenticationinapp/authenticationinapp.h"
#include "../../src/networkrequest.h"
#include "../../src/tasks/authenticate/taskauthenticate.h"
#include "../../src/tasks/function/taskfunction.h"

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
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  QVERIFY(!!aia);
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  EventLoop loop;
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia->checkAccount(emailAddress);
  QCOMPARE(aia->state(), AuthenticationInApp::StateCheckingAccount);

  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    QVERIFY(aia->state() != AuthenticationInApp::StateSignIn);
    if (aia->state() == AuthenticationInApp::StateSignUp) {
      loop.exit();
    }
  });
  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateSignUp);

  // Password
  aia->setPassword(PASSWORD);

  if (m_totpCreation) {
    aia->enableTotpCreation();
  }

  // Sign-up
  aia->signUp();
  QCOMPARE(aia->state(), AuthenticationInApp::StateSigningUp);

  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() ==
        AuthenticationInApp::StateVerificationSessionByEmailNeeded) {
      loop.exit();
    }
  });
  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(),
           AuthenticationInApp::StateVerificationSessionByEmailNeeded);

  // Email verification - with invalid code
  aia->verifySessionEmailCode("000000");
  QCOMPARE(aia->state(), AuthenticationInApp::StateVerifyingSessionEmailCode);

  connect(aia, &AuthenticationInApp::errorOccurred,
          [&](AuthenticationInApp::ErrorType error) {
            if (error ==
                AuthenticationInApp::ErrorInvalidOrExpiredVerificationCode) {
              loop.exit();
            }
          });
  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(),
           AuthenticationInApp::StateVerificationSessionByEmailNeeded);

  // Email verification - with valid code
  QString code = fetchSessionCode();
  QVERIFY(!code.isEmpty());
  aia->verifySessionEmailCode(code);
  QCOMPARE(aia->state(), AuthenticationInApp::StateVerifyingSessionEmailCode);

  QUrl finalUrl;
  connect(aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });

  if (m_totpCreation) {
    waitForTotpCodes();
  }

  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  // The account is not active yet. So, let's check the final URL.
  QVERIFY(
      (finalUrl.host() == "stage-vpn.guardian.nonprod.cloudops.mozgcp.net" &&
       finalUrl.path() == "/vpn/client/login/success") ||
      (finalUrl.host() == "www-dev.allizom.org" &&
       finalUrl.path() == "/en-US/products/vpn/"));
  qDebug() << finalUrl.path();
}

void TestSignUpAndIn::signIn() {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  QVERIFY(!!aia);
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run();

  EventLoop loop;
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia->checkAccount(emailAddress);
  QCOMPARE(aia->state(), AuthenticationInApp::StateCheckingAccount);

  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateSignIn) {
      loop.exit();
    }
  });

  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateSignIn);

  // Password
  aia->setPassword(PASSWORD);

  // Sign-in
  aia->signIn();
  QCOMPARE(aia->state(), AuthenticationInApp::StateSigningIn);

  // The next step can be tricky: totp, or unblocked code, or success
  if (m_totpCreation) {
    waitForTotpCodes();
  }

  bool wrongUnblockCodeSent = false;

  connect(aia, &AuthenticationInApp::errorOccurred,
          [this](AuthenticationInApp::ErrorType error) {
            if (error == AuthenticationInApp::ErrorInvalidUnblockCode) {
              qDebug() << "Invalid unblock code. Sending a good one";

              AuthenticationInApp* aia = AuthenticationInApp::instance();
              QCOMPARE(aia->state(),
                       AuthenticationInApp::StateUnblockCodeNeeded);

              // We do not receive the email each time.
              aia->resendUnblockCodeEmail();

              QString code = fetchUnblockCode();
              QVERIFY(!code.isEmpty());
              aia->verifyUnblockCode(code);
              QCOMPARE(aia->state(),
                       AuthenticationInApp::StateVerifyingUnblockCode);
            }
          });

  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (!wrongUnblockCodeSent &&
        aia->state() == AuthenticationInApp::StateUnblockCodeNeeded) {
      aia->verifyUnblockCode("000000");
      QCOMPARE(aia->state(), AuthenticationInApp::StateVerifyingUnblockCode);

      wrongUnblockCodeSent = true;
    }
  });

  QUrl finalUrl;
  connect(aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });

  loop.exec();
  disconnect(aia, nullptr, nullptr, nullptr);

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

  AuthenticationInApp* aia = AuthenticationInApp::instance();

  connect(aia, &AuthenticationInApp::errorOccurred,
          [this](AuthenticationInApp::ErrorType error) {
            if (error == AuthenticationInApp::ErrorInvalidTotpCode) {
              qDebug() << "Invalid code. Let's send the right one";

              AuthenticationInApp* aia = AuthenticationInApp::instance();
              QCOMPARE(
                  aia->state(),
                  AuthenticationInApp::StateVerificationSessionByTotpNeeded);

              QCOMPARE(oath_init(), OATH_OK);

              char otp[/* length + 1 */ 7] = {};
              QCOMPARE(oath_totp_generate(m_totpSecret.data(),
                                          m_totpSecret.length(), time(nullptr),
                                          OATH_TOTP_DEFAULT_TIME_STEP_SIZE,
                                          OATH_TOTP_DEFAULT_START_TIME, 6, otp),
                       OATH_OK);

              qDebug() << "Code:" << otp;
              aia->verifySessionTotpCode(otp);
              QCOMPARE(aia->state(),
                       AuthenticationInApp::StateVerifyingSessionTotpCode);
              m_sendWrongTotpCode = true;
            }
          });

  connect(aia, &AuthenticationInApp::unitTestTotpCodeCreated,
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

  connect(aia, &AuthenticationInApp::stateChanged, [this]() {
    AuthenticationInApp* aia = AuthenticationInApp::instance();
    qDebug() << "Send wrong code:" << m_sendWrongTotpCode;

    if (m_sendWrongTotpCode &&
        aia->state() ==
            AuthenticationInApp::StateVerificationSessionByTotpNeeded) {
      m_sendWrongTotpCode = false;
      qDebug() << "Code required. Let's write a wrong code first.";
      aia->verifySessionTotpCode("123456");
      QCOMPARE(aia->state(),
               AuthenticationInApp::StateVerifyingSessionTotpCode);
    }
  });
}

QString TestSignUpAndIn::fetchCode(const QString& code) {
  while (true) {
    QString url = "http://restmail.net/mail/";
    url.append(m_emailAccount);

    // In theory, network requests should be executed by tasks, but for this
    // test we do an hack.
    TaskFunction dummyTask([] {});

    NetworkRequest* nr = NetworkRequest::createForGetUrl(&dummyTask, url);

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
