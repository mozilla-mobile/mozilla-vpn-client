/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testenv.h"

#include "context/env.h"

#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif

void TestEnv::osVersion() {
  QString expected;
#ifdef MZ_WINDOWS
  expected = WindowsUtils::windowsVersion();
#else
  expected = QSysInfo::productVersion();
#endif
  QString actual = Env::osVersion();
  QCOMPARE(actual, expected);
}

void TestEnv::architecture() {
  QString expected = QSysInfo::currentCpuArchitecture();
  QString actual = Env::architecture();
  QCOMPARE(actual, expected);
}

QTEST_MAIN(TestEnv);
