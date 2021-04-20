/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnoop.h"

void TestNoop::noop() {
  // No operations here. We test that the app starts and stops.
  QVERIFY(s_nativeMessagingProcess);
}

static TestNoop s_testNoop;
