/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testipfinder.h"

#include <QHostInfo>

#include "context/constants.h"
#include "helper.h"
#include "settings/settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/ipfinder/taskipfinder.h"

void TestIpFinder::ipv4AndIpv6() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;

  settingsHolder.setToken("TOKEN!");

  TestHelper::networkConfig.clear();

  QEventLoop loop;
  QUrl url(Constants::apiBaseUrl());

  bool ipv4Expected = false;
  bool ipv6Expected = false;

  QHostInfo::lookupHost(url.host(), [&](const QHostInfo& hostInfo) {
    if (hostInfo.error() == QHostInfo::NoError) {
      for (const QHostAddress& address : hostInfo.addresses()) {
        if (address.isNull() || address.isBroadcast()) continue;

        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
          TestHelper::networkConfig.append(TestHelper::NetworkConfig(
              TestHelper::NetworkConfig::Success,
              QString("{\"ip\":\"42\", \"country\": \"123\"}").toUtf8()));
          ipv4Expected = true;
          continue;
        }

        if (address.protocol() == QAbstractSocket::IPv6Protocol) {
          TestHelper::networkConfig.append(TestHelper::NetworkConfig(
              TestHelper::NetworkConfig::Success,
              QString("{\"ip\":\"43\", \"country\": \"123\"}").toUtf8()));
          ipv6Expected = true;
          continue;
        }
      }

      loop.exit();
    }
  });
  loop.exec();

  if (!ipv4Expected && !ipv6Expected) {
    return;
  }

  TaskIPFinder* ipFinder = new TaskIPFinder();

  connect(
      ipFinder, &TaskIPFinder::operationCompleted, ipFinder,
      [&](const QString& ipv4, const QString& ipv6, const QString& country) {
        if (ipv4Expected) {
          QVERIFY(ipv4 == "43" || ipv4 == "42");
        }

        if (ipv6Expected) {
          QVERIFY(ipv6 == "43" || ipv6 == "42");
        }

        QVERIFY(ipv4 != ipv6);
        QCOMPARE(country, "123");
        loop.exit();
      });

  ipFinder->run();
  loop.exec();
}

static TestIpFinder s_testIpFinder;
