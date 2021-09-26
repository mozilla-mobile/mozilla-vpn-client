/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testpasswordvalidation.h"
#include "../../src/authenticationinapp/authenticationinapp.h"
#include "../../src/authenticationinapp/incrementaldecoder.h"
#include "../../src/tasks/authenticate/taskauthenticate.h"

#include <QDateTime>
#include <QDebug>
#include <QEventLoop>
#include <QTest>

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

void TestPasswordValidation::incrementalDecoder_data() {
  QTest::addColumn<QString>("buffer");
  QTest::addColumn<QString>("input");
  QTest::addColumn<IncrementalDecoder::Result>("result");

  QTest::addRow("empty") << ""
                         << "" << IncrementalDecoder::MatchNotFound;

  {
    QString buffer;
    buffer.append("0word\n3ld");
    QTest::addRow("match 1")
        << buffer << "world" << IncrementalDecoder::MatchFound;
  }

  {
    QString buffer;
    buffer.append("0entry\n52\n61\n0out_of_order");
    QTest::addRow("match 2")
        << buffer << "entry" << IncrementalDecoder::MatchFound;
    QTest::addRow("match 3")
        << buffer << "entry2" << IncrementalDecoder::MatchFound;
    QTest::addRow("match 4")
        << buffer << "entry21" << IncrementalDecoder::MatchFound;
    QTest::addRow("match 5")
        << buffer << "out_of_order" << IncrementalDecoder::MatchFound;
  }
}

void TestPasswordValidation::incrementalDecoder() {
  QFETCH(QString, buffer);
  QFETCH(QString, input);
  QFETCH(IncrementalDecoder::Result, result);

  QTextStream stream(&buffer);

  IncrementalDecoder id(this);
  QCOMPARE(id.match(stream, input), result);
}

void TestPasswordValidation::commonPasswords_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");

  QTest::addRow("common 1") << "12345678" << false;
  QTest::addRow("common 2") << "12345678!!" << true;

  // Empty password, is not a common password :)
  QTest::addRow("common 3") << "" << true;
}

void TestPasswordValidation::commonPasswords() {
  QFETCH(QString, input);
  QFETCH(bool, result);

  QCOMPARE(AuthenticationInApp::validatePasswordCommons(input), result);
}

void TestPasswordValidation::passwordLength_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << "" << false;
  QTest::addRow("common") << "123456" << false;
  QTest::addRow("common") << "12345678!!" << true;
}

void TestPasswordValidation::passwordLength() {
  QFETCH(QString, input);
  QFETCH(bool, result);

  QCOMPARE(AuthenticationInApp::validatePasswordLength(input), result);
}

void TestPasswordValidation::emailPassword() {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  QVERIFY(!!aia);
  disconnect(aia, nullptr, nullptr, nullptr);

  QCOMPARE(aia->state(), AuthenticationInApp::StateInitializing);

  // Starting the authentication flow.
  TaskAuthenticate task(Core::AuthenticationInApp);
  task.run();

  EventLoop loop;
  connect(aia, &AuthenticationInApp::stateChanged, [&]() {
    if (aia->state() == AuthenticationInApp::StateStart) {
      loop.exit();
    }
  });
  loop.exec();
  QCOMPARE(aia->state(), AuthenticationInApp::StateStart);

  QString emailAddress("vpn.test.auth.password.");
  emailAddress.append(QString::number(QDateTime::currentSecsSinceEpoch()));
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

  // Password checks
  QCOMPARE(aia->validatePasswordEmail(emailAddress), false);
  QCOMPARE(aia->validatePasswordEmail("hello world!"), true);
}
