/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "../../src/loghandler.h"
#include "../../src/settingsholder.h"
#include "constants.h"
#include "helper.h"
#include "l18nstrings.h"

QVector<TestHelper::NetworkConfig> TestHelper::networkConfig;
MozillaVPN::State TestHelper::vpnState = MozillaVPN::StateInitialize;
Controller::State TestHelper::controllerState = Controller::StateInitializing;
MozillaVPN::UserState TestHelper::userState = MozillaVPN::UserNotAuthenticated;
QVector<QObject*> TestHelper::testList;

QObject* TestHelper::findTest(const QString& name) {
  for (QObject* obj : TestHelper::testList) {
    const QMetaObject* meta = obj->metaObject();
    if (meta->className() == name) {
      return obj;
    }
  }

  return nullptr;
}

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

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  pe.insert("LANG", "en");
  pe.insert("LANGUAGE", "en");

  QCoreApplication a(argc, argv);

  int failures = 0;

  L18nStrings::initialize();
  LogHandler::enableDebug();

  // If arguments were passed, then run a subset of tests.
  QStringList args = a.arguments();
  if (args.count() > 1) {
    args.removeFirst();
    for (const QString& x : args) {
      QObject* obj = TestHelper::findTest(x);
      if (obj == nullptr) {
        qWarning() << "No such test found:" << x;
        ++failures;
        continue;
      }
      int result = QTest::qExec(obj);
      if (result != 0) {
        ++failures;
      }
    }
  } else {
    // Otherwise, run all the tests.
    for (QObject* obj : TestHelper::testList) {
      int result = QTest::qExec(obj);
      if (result != 0) {
        ++failures;
      }
    }
  }

  return failures;
}
