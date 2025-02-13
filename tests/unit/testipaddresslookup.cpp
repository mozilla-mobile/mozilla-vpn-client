/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testipaddresslookup.h"

#include "helper.h"
#include "ipaddresslookup.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

void TestIpAddressLookup::checkIpAddressFailure() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;

  settingsHolder.setToken("TOKEN!");

  IpAddressLookup ial;
  ial.reset();

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));

  QEventLoop loop;
  connect(&ial, &IpAddressLookup::ipAddressChecked, &ial, [&] { loop.exit(); });

  ial.updateIpAddress();
  loop.exec();
}

void TestIpAddressLookup::checkIpAddressSucceess_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<QString>("ipAddress");
  QTest::addColumn<bool>("signal");

  QTest::addRow("invalid") << QByteArray("") << "vpn.connectionInfo.loading"
                           << false;

  QJsonObject json;
  QTest::addRow("empty") << QJsonDocument(json).toJson()
                         << "vpn.connectionInfo.loading" << false;

  json.insert("ip", 42);
  QTest::addRow("invalid ip")
      << QJsonDocument(json).toJson() << "vpn.connectionInfo.loading" << false;

  json.insert("ip", "42");
  QTest::addRow("valid ip") << QJsonDocument(json).toJson() << "42" << true;
}

void TestIpAddressLookup::checkIpAddressSucceess() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;

  settingsHolder.setToken("TOKEN!");

  IpAddressLookup ial;
  ial.reset();

  QSignalSpy spy(&ial, &IpAddressLookup::ipv4AddressChanged);

  TestHelper::networkConfig.clear();

  QFETCH(QByteArray, json);
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));

  QEventLoop loop;
  connect(&ial, &IpAddressLookup::ipAddressChecked, &ial, [&] {
    QFETCH(QString, ipAddress);
    QCOMPARE(ial.ipv4Address(), ipAddress);

    QFETCH(bool, signal);
    QCOMPARE(spy.count(), signal ? 1 : 0);
    loop.exit();
  });

  ial.updateIpAddress();
  loop.exec();
}

// static TestIpAddressLookup s_testIpAddressLookup;
