/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "appconstants.h"
#include "helper.h"
<<<<<<< HEAD
=======
#include "i18nstrings.h"
>>>>>>> 39e260398 (Rename VPNL18n to VPNI18n (#5994))
#include "leakdetector.h"
#include "loghandler.h"
#include "settingsholder.h"

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
#ifdef MZ_DEBUG
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

  QCoreApplication app(argc, argv);

  int failures = 0;

  LogHandler::enableStderr();

  // If arguments were passed, then run a subset of tests.
  QStringList args = app.arguments();
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
