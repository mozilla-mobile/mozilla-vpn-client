/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "test_illegal_invoke.h"

void TestIllegalNoop::setup() {
  // We will have a running instance with correct params
  // to begin with, let's close that.
  QVERIFY(s_nativeMessagingProcess);
  killNativeMessaging();
  QVERIFY(s_nativeMessagingProcess == nullptr);
}

void TestIllegalNoop::no_params() {
  // // Now we verify that it does not start
  runNativeMessaging(s_app,QStringList());
  s_nativeMessagingProcess->waitForFinished();
  s_nativeMessagingProcess = nullptr;
  QVERIFY(s_last_exit_code == 1);
}

void TestIllegalNoop::wrong_param() {
  QStringList args;
  args.append("aaaa");
  args.append("bbbb");
  // // Now we verify that it does not start
  runNativeMessaging(s_app, args);
  s_nativeMessagingProcess->waitForFinished();
  s_nativeMessagingProcess = nullptr;
  QVERIFY(s_last_exit_code == 1);
}

void TestIllegalNoop::partially_wrong_param() {
  QStringList args;
  args.append("/path/to/something/nothing/checks/this");
  args.append("@testpilot-containers");
  args.append("this should not be here");
  // // Now we verify that it does not start
  runNativeMessaging(s_app, args);
  s_nativeMessagingProcess->waitForFinished();
  s_nativeMessagingProcess = nullptr;
  QVERIFY(s_last_exit_code == 1);
}

static TestIllegalNoop s_testNoop;
