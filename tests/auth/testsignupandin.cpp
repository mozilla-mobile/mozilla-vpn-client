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

TestSignUpAndIn::TestSignUpAndIn(const QString& pattern) {
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
  task.run(MozillaVPN::instance());

  EventLoop loop;
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia->checkAccount(emailAddress);
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    QVERIFY(aia->state() != AuthenticationInApp::StateSignIn);
    if (aia->state() == AuthenticationInApp::StateSignUp) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(), AuthenticationInApp::StateSignUp);

  // Password
  aia->setPassword(PASSWORD);

  // Sign-up
  aia->signUp();
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() ==
        AuthenticationInApp::StateVerificationSessionByEmailNeeded) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(),
           AuthenticationInApp::StateVerificationSessionByEmailNeeded);

  // Email verification
  QString code = fetchSessionCode();
  QVERIFY(!code.isEmpty());
  aia->verifySessionEmailCode(code);

  QUrl finalUrl;
  connect(aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });
  loop.exec();

  // The account is not active yet. So, let's check the final URL.
  QCOMPARE(finalUrl.host(), "www-dev.allizom.org");
  QCOMPARE(finalUrl.path(), "/en-US/products/vpn/");
}

void TestSignUpAndIn::signIn() {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  QVERIFY(!!aia);
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(MozillaVPN::AuthenticationInApp);
  task.run(MozillaVPN::instance());

  EventLoop loop;
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateUnblockCodeNeeded) {
      // We do not receive the email each time.
      AuthenticationInApp::instance()->resendUnblockCodeEmail();

      QString code = fetchUnblockCode();
      QVERIFY(!code.isEmpty());
      aia->setUnblockCodeAndContinue(code);
      return;
    }

    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(), AuthenticationInApp::StateStart);

  QString emailAddress(m_emailAccount);
  emailAddress.append("@restmail.net");

  // Account
  aia->checkAccount(emailAddress);
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateSignIn) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(), AuthenticationInApp::StateSignIn);

  // Password
  aia->setPassword(PASSWORD);

  // Sign-in
  aia->signIn();

  QUrl finalUrl;
  connect(aia, &AuthenticationInApp::unitTestFinalUrl,
          [&](const QUrl& url) { finalUrl = url; });
  connect(&task, &Task::completed, [&]() {
    qDebug() << "Task completed";
    loop.exit();
  });
  loop.exec();

  // The account is not active yet. So, let's check the final URL.
  QCOMPARE(finalUrl.host(), "www-dev.allizom.org");
  QCOMPARE(finalUrl.path(), "/en-US/products/vpn/");
}

QString TestSignUpAndIn::fetchSessionCode() {
  return fetchCode("x-verify-short-code");
}

QString TestSignUpAndIn::fetchUnblockCode() {
  return fetchCode("x-unblock-code");
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
