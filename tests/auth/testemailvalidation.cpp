/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testemailvalidation.h"
#include "../../src/authenticationinapp/authenticationinapp.h"

#include <QDebug>
#include <QTest>

void TestEmailValidation::basic_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << "" << false;
  QTest::addRow("no @") << "hello world" << false;
  QTest::addRow("too many @") << "hello@world@!" << false;

  QString emailAddress("a@");
  for (int i = 0; i < 256; ++i) emailAddress.append("a");
  QTest::addRow("domain too big") << emailAddress << false;

  QTest::addRow("invalid part 0") << ",@a" << false;
  QTest::addRow("invalid part 1") << "a@^" << false;

  QTest::addRow("all good") << "a@b.c" << true;
}

void TestEmailValidation::basic() {
  QFETCH(QString, input);
  QFETCH(bool, result);

  QCOMPARE(AuthenticationInApp::validateEmailAddress(input), result);
}
