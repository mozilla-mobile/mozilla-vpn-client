/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/authenticationinapp/authenticationinapp.h"
#include "../../src/constants.h"
#include "../../src/featurelist.h"
#include "../../src/leakdetector.h"
#include "../../src/settingsholder.h"
#include "../../src/simplenetworkmanager.h"

#include "testemailvalidation.h"
#include "testpasswordvalidation.h"
#include "testsignupandin.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

int main(int argc, char* argv[]) {
#ifdef QT_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

  Constants::setStaging();

  QCoreApplication a(argc, argv);

  SimpleNetworkManager snm;
  SettingsHolder settingsHolder;
  FeatureList::instance()->initialize();

  int failures = 0;
  TestEmailValidation tev;
  failures += QTest::qExec(&tev);

  TestPasswordValidation tpv;
  failures += QTest::qExec(&tpv);

  TestSignUpAndIn tsuTotp("vpn.auth.test.", true /* totp creation */);
  failures += QTest::qExec(&tsuTotp);

  TestSignUpAndIn tsu("vpn.auth.test.");
  failures += QTest::qExec(&tsu);

  TestSignUpAndIn tsuBlocked("block.vpn.auth.test.");
  failures += QTest::qExec(&tsuBlocked);

  TestSignUpAndIn tsuSync("sync.vpn.auth.test.");
  failures += QTest::qExec(&tsuSync);

  return failures;
}
