/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbaseadapter.h"

#include <QHostAddress>
#include <QJsonObject>
#include <QTest>

#include "baseadapter.h"

QTEST_MAIN(TestBaseAdapter)

struct TestCase {
  QHostAddress addr;
  bool excepted;
};

class TestAdapter : public WebExtension::BaseAdapter {
 public:
  TestAdapter() : BaseAdapter(nullptr){};

  void addCommand(RequestType cmd) { m_commands.append(cmd); }
};

void TestBaseAdapter::testOnMessage() {
  TestAdapter test;

  struct CaseSetup {
    QString key;
    bool expectedToRun;
  };

  QList<CaseSetup> testCases{
      {"testing", true}, {"not_testing", false}, {"", false}};
  bool command_run = false;
  test.addCommand(WebExtension::BaseAdapter::RequestType{
      "testing", [&](const QJsonObject& args) {
        command_run = true;
        return QJsonObject();
      }});
  for (auto& testCase : testCases) {
    command_run = false;

    QJsonObject inputArgs;
    inputArgs["t"] = testCase.key;
    test.onMessage(inputArgs);

    QCOMPARE(command_run, testCase.expectedToRun);
  }
}

void TestBaseAdapter::testEmitsMessage() {
  TestAdapter test;
  QEventLoop loop;
  QJsonObject inputArgs;
  inputArgs["t"] = "testing";
  test.addCommand(WebExtension::BaseAdapter::RequestType{
      "testing", [&](const QJsonObject& args) {
        QJsonObject out;
        out["ok"] = true;
        return out;
      }});
  bool gotOkMessage = false;
  connect(&test, &WebExtension::BaseAdapter::onOutgoingMessage,
          [&](QJsonObject val) { gotOkMessage = val["ok"].toBool(); });
  test.onMessage(inputArgs);

  loop.processEvents();

  QCOMPARE(gotOkMessage, true);
}

void TestBaseAdapter::testEmptyMessageEmitsInvalidRequest() {
  TestAdapter test;
  QEventLoop loop;
  QJsonObject emptyInputArgs;
  connect(&test, &WebExtension::BaseAdapter::onOutgoingMessage,
          [&](QJsonObject val) {
            auto messageType = val["t"].toString();
            QCOMPARE(messageType, "invalidRequest");
          });
  test.onMessage(emptyInputArgs);
  loop.processEvents();
}

void TestBaseAdapter::testTypeViolationEmitsInvalidRequest() {
  TestAdapter test;
  QEventLoop loop;
  QJsonObject typeViolationArgs;
  // Add a "type" but have the property violate the type.
  typeViolationArgs["t"] = 43;
  connect(&test, &WebExtension::BaseAdapter::onOutgoingMessage,
          [&](QJsonObject val) {
            auto messageType = val["t"].toString();
            QCOMPARE(messageType, "invalidRequest");
          });
  test.onMessage(typeViolationArgs);
  loop.processEvents();
}

void TestBaseAdapter::testUnknownCommandEmitsError() {
  TestAdapter test;
  QEventLoop loop;
  QJsonObject typeViolationArgs;
  typeViolationArgs["t"] = "this-command-does-not-exist";
  connect(&test, &WebExtension::BaseAdapter::onOutgoingMessage,
          [&](QJsonObject val) {
            auto messageType = val["t"].toString();
            QCOMPARE(messageType, "error");
          });
  test.onMessage(typeViolationArgs);
  loop.processEvents();
}
