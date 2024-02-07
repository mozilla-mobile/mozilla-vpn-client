/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testBaseAdapter.h"

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
