/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "test_illegal_invoke.h"

void TestIllegalNoop::no_params() {
  // // Now we verify that it does not start
  runNativeMessaging(QStringList());
  m_nativeMessagingProcess.waitForFinished();
  QCOMPARE(m_nativeMessagingProcess.exitCode(), 1);
}

void TestIllegalNoop::wrong_param() {
  QStringList args;
  args.append("aaaa");
  args.append("bbbb");
  // // Now we verify that it does not start
  runNativeMessaging(args);
  m_nativeMessagingProcess.waitForFinished();
  QCOMPARE(m_nativeMessagingProcess.exitCode(), 1);
}

void TestIllegalNoop::partially_wrong_param() {
  QStringList args;
  args.append("/path/to/something/nothing/checks/this");
  args.append("@testpilot-containers");
  args.append("this should not be here");
  // // Now we verify that it does not start
  runNativeMessaging(args);
  m_nativeMessagingProcess.waitForFinished();
  QCOMPARE(m_nativeMessagingProcess.exitCode(), 1);
}

static TestIllegalNoop s_testNoop;
