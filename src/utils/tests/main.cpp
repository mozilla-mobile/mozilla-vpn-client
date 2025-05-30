/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QObject>
#include <QVector>
#include <QtTest/QtTest>

#include "loghandler.h"
#include "testhelper.h"

int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif
  QCoreApplication::setApplicationName("Mozilla VPN Unit Tests");
  QCoreApplication::setOrganizationName("Mozilla Testing");
  LogHandler::instance()->setStderr(true);
  QCoreApplication app(argc, argv);

  // Execute the tests
  int numFails = 0;
  int numTests = 0;
  for (auto test = TestRegistration::list(); test != nullptr; test = test->m_next) {
    auto obj = reinterpret_cast<QObject*>(test->m_meta->metaType().create());
    numTests++;
    if (QTest::qExec(obj) != 0) {
      numFails++;
    }
  }
  return numFails;
}
