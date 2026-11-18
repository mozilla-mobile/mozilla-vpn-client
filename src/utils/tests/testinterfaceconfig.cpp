/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testinterfaceconfig.h"

#include <QJsonDocument>
#include <QtTest/QtTest>

#include "interfaceconfig.h"

void TestInterfaceConfig::basic() {
  InterfaceConfig config;

  // The config should be invalid until a device configuration has been set.
  QVERIFY(!config.isValid());

  config.m_privateKey = "TestPrivateKey";
  QVERIFY(!config.isValid());
  config.m_publicKey = "TestPublicKey";
  QVERIFY(!config.isValid());
  config.m_deviceIpv4Address = "10.123.45.66/32";
  QVERIFY(!config.isValid());
  config.m_deviceIpv6Address = "fc02::dead:beef/128";
  QVERIFY(config.isValid());
}

void TestInterfaceConfig::json_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<InterfaceConfig>("expect");

  // Check some malformed JSON cases
  QTest::addRow("empty") << QString("{}").toUtf8() << InterfaceConfig();
  QTest::addRow("malformed") << QString("{").toUtf8() << InterfaceConfig();
  QTest::addRow("junk") << QString("{\"test\": 12345}").toUtf8() << InterfaceConfig();
  QTest::addRow("array") << QString("[]").toUtf8() << InterfaceConfig();

  auto encode = [](const InterfaceConfig& cfg) {
    return QJsonDocument(cfg.toJson()).toJson();
  };

  // Start with a minimal configuration.
  InterfaceConfig config;
  config.m_hopType = InterfaceConfig::SingleHop;
  config.m_privateKey = "TestPrivateKey";
  config.m_publicKey = "TestPublicKey";
  config.m_deviceIpv4Address = "10.123.45.66/32";
  config.m_deviceIpv6Address = "fc02::dead:beef/128";

  config.m_serverPublicKey = "ServerPublicKey";
  config.m_serverIpv4Gateway = "10.123.0.1";
  config.m_serverIpv4AddrIn = "127.0.0.1";
  config.m_serverPort = 51820;
  config.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  QTest::addRow("minimal") << encode(config) << config;

  // Test that encoding still succeeds even when an extra "type" field is
  // present. Some of the daemon impementations do this to set an action.
  QJsonObject obj = config.toJson();
  obj.insert("type", "example");
  QTest::addRow("extra type") << QJsonDocument(obj).toJson() << config;

  // Build up the server configuration.
  config.m_allowedIPAddressRanges.append(IPAddress("10.64.0.0/16"));
  QTest::addRow("allowedips extra") << encode(config) << config;
  config.m_serverIpv6Gateway = "fc02::1";
  QTest::addRow("server gateway v6") << encode(config) << config;
  config.m_serverIpv6AddrIn = "::1";
  QTest::addRow("server addr v6") << encode(config) << config;
  config.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("fc02::/64"));
  QTest::addRow("allowedips v6") << encode(config) << config;
  config.m_dnsServer = "dns.example.com";
  QTest::addRow("dns servers") << encode(config) << config;
  config.m_vpnDisabledApps << "example-app-1";
  config.m_vpnDisabledApps << "example-app-2";
  QTest::addRow("split tunnel") << encode(config) << config;
  config.m_obfuscationMethod = Server::ObfuscationMethod::LWO;
  QTest::addRow("lwo obfuscation") << encode(config) << config;
  config.m_lwoVersion = 2;
  QTest::addRow("lwo version") << encode(config) << config;
}

// Test that we can encode/decode an interface config through JSON and
// get the same thing we started with.
void TestInterfaceConfig::json() {
  QFETCH(QByteArray, json);
  QFETCH(InterfaceConfig, expect);

  // Decode the provided JSON
  QJsonDocument jsDocument = QJsonDocument::fromJson(json);
  InterfaceConfig decoded(jsDocument.object());

  if (!expect.isValid()) {
    // Just check that the result is also invalid.
    QVERIFY(!decoded.isValid());
    return;
  }

  compare(decoded, expect);

  // Valid configurations should also be unchanged after a roudntrip through
  // JSON encoding and back.
  QJsonObject obj = expect.toJson();
  InterfaceConfig roundtrip(obj);
  compare(roundtrip, expect);
}

void TestInterfaceConfig::compare(const InterfaceConfig& a, const InterfaceConfig &b) {
  QCOMPARE(a.m_hopType, b.m_hopType);

  // Device configuration
  QCOMPARE(a.m_privateKey, b.m_privateKey);
  QCOMPARE(a.m_publicKey, b.m_publicKey);
  QCOMPARE(a.m_deviceIpv4Address, b.m_deviceIpv4Address);
  QCOMPARE(a.m_deviceIpv6Address, b.m_deviceIpv6Address);

  // Wireguard server configuration.
  QCOMPARE(a.m_serverPublicKey, b.m_serverPublicKey);
  QCOMPARE(a.m_serverIpv4Gateway, b.m_serverIpv4Gateway);
  QCOMPARE(a.m_serverIpv6Gateway, b.m_serverIpv6Gateway);
  QCOMPARE(a.m_serverIpv4AddrIn, b.m_serverIpv4AddrIn);
  QCOMPARE(a.m_serverIpv4AddrIn, b.m_serverIpv4AddrIn);
  QCOMPARE(a.m_serverIpv6AddrIn, b.m_serverIpv6AddrIn);

  // These are localized city names. They are used for iOS widgets and other
  // situations where city name is needed, but the client is not running.
  QCOMPARE(a.m_entryCity, b.m_entryCity);
  QCOMPARE(a.m_exitCity, b.m_exitCity);

  // Misc configuration
  QCOMPARE(a.m_dnsServer, b.m_dnsServer);
  QCOMPARE(a.m_vpnDisabledApps, b.m_vpnDisabledApps);
  QCOMPARE(a.m_obfuscationMethod, b.m_obfuscationMethod);
  QCOMPARE(a.m_lwoVersion, b.m_lwoVersion);

  // The allowed ip list may be sorted, so take a bit more care.
  QCOMPARE(a.m_allowedIPAddressRanges.count(), b.m_allowedIPAddressRanges.count());
  for (auto i : a.m_allowedIPAddressRanges) {
    QVERIFY(b.m_allowedIPAddressRanges.contains(i));
  }
  for (auto i : b.m_allowedIPAddressRanges) {
    QVERIFY(a.m_allowedIPAddressRanges.contains(i));
  }
}
