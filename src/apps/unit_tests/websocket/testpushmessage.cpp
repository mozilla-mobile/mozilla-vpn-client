/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.orgß/MPL/2.0/. */

#include "testpushmessage.h"

#include "helper.h"
#include "websocket/pushmessage.h"

void TestPushMessage::tst_invalidMessagesAreIgnored() {
  QString msg = "invalid message";
  PushMessage message(msg);
  // Nothing was executed.
  QVERIFY(!message.executeAction());
}

void TestPushMessage::tst_validMessagesAreParsedAndExecuted() {
  PushMessage::registerPushMessageType(
      "TEST_MESSAGE", [](const QJsonObject& payload) -> bool { return true; });

  QString msg = "{\"type\": \"TEST_MESSAGE\"}";
  PushMessage message(msg);
  message.executeAction();
  QVERIFY(message.executeAction());
}

static TestPushMessage s_testPushMessage;
