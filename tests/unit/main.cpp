/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "../../src/loghandler.h"
#include "../../src/settingsholder.h"
#include "constants.h"
#include "helper.h"

QVector<TestHelper::NetworkConfig> TestHelper::networkConfig;
MozillaVPN::State TestHelper::vpnState = MozillaVPN::StateInitialize;
Controller::State TestHelper::controllerState = Controller::StateInitializing;
QVector<QObject*> TestHelper::testList;

TestHelper::TestHelper() { testList.append(this); }

int main(int argc, char* argv[]) {
#ifdef MVPN_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif
  {
    SettingsHolder settingsHolder;
    Constants::setStaging();
  }

  QCoreApplication a(argc, argv);

  int failures = 0;

  LogHandler::enableDebug();

  for (QObject* obj : TestHelper::testList) {
    int result = QTest::qExec(obj);
    if (result != 0) {
      ++failures;
    }
  }

  return failures;
}
