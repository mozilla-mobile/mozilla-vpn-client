/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testipfinder.h"
#include "../../src/tasks/ipfinder/taskipfinder.h"
#include "../../src/settingsholder.h"
#include "helper.h"

void TestIpFinder::ipv4AndIpv6() {
  SettingsHolder::instance();

  TaskIPFinder* ipFinder = new TaskIPFinder();

  TestHelper::networkConfig.clear();

  QEventLoop loop;
  connect(
      ipFinder, &TaskIPFinder::operationCompleted,
      [&](const QString& ipv4, const QString& ipv6, const QString& country) {
        QVERIFY(ipv4 == "43" || ipv4 == "42");
        QVERIFY(ipv6 == "43" || ipv6 == "42");
        QVERIFY(ipv4 != ipv6);
        QCOMPARE(country, "123");
        loop.exit();
      });

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success,
      QString("{\"ip\":\"42\", \"country\": \"123\"}").toUtf8()));
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success,
      QString("{\"ip\":\"43\", \"country\": \"123\"}").toUtf8()));

  ipFinder->run();
  loop.exec();
}

static TestIpFinder s_testIpFinder;
