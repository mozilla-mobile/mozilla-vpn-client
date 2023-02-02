/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QtTest/QtTest>

#include "appconstants.h"
#include "authenticationinapp/authenticationinapp.h"
#include "leakdetector.h"
#include "loghandler.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "testemailvalidation.h"
#include "testpasswordvalidation.h"
#include "testsignupandin.h"
#include "glean/glean.h"

int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

  SettingsHolder settingsHolder;
  AppConstants::setStaging();

  QCoreApplication a(argc, argv);

  SimpleNetworkManager snm;

  settingsHolder.setFeaturesFlippedOn(QStringList{
      "inAppAccountCreate", "inAppAuthentication", "accountDeletion"});

  LogHandler::enableStderr();
  VPNGlean::registerLogHandler(LogHandler::rustMessageHandler);

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
