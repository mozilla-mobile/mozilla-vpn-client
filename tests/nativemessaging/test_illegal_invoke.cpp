/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "test_illegal_invoke.h"

void TestIllegalNoop::illegal_noop() {
  // We test that the app starts and stops.
  QVERIFY(s_nativeMessagingProcess);
  killNativeMessaging();
  QVERIFY(!s_nativeMessagingProcess);
  // Now we verify that it does not start
  runNativeMessaging(s_app,QStringList());
  QVERIFY(!s_nativeMessagingProcess);
}

static TestIllegalNoop s_testNoop;
