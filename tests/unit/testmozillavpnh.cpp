/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmozillavpnh.h"

// Although the following two tests are trivial checks of the mozillavpn.h logic
// for osVersion and architecture; breaking or changing these values would cause
// our glean analytics to not be comparable over time so it's important to have
// this check and balance.
void TestMozillaVPNh::osVersion() {
  QString expected;
#ifdef MVPN_WINDOWS
  expected = WindowsCommons::WindowsVersion();
#else
  expected = QSysInfo::productVersion();
#endif
  QString actual = Env::osVersion();
  QCOMPARE(actual, expected);
}

void TestMozillaVPNh::architecture() {
  QString expected = QSysInfo::currentCpuArchitecture();
  QString actual = Env::architecture();
  QCOMPARE(actual, expected);
}

static TestMozillaVPNh s_testMozillaVPNh;
