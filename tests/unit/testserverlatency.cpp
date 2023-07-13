/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserverlatency.h"

#include <QDateTime>
#include <QJsonObject>

#include "constants.h"
#include "feature.h"
#include "models/location.h"
#include "models/servercity.h"
#include "serverlatency.h"
#include "settingsholder.h"

void TestServerLatency::init() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"serverConnectionScore"});
  QVERIFY(settingsHolder.featuresFlippedOn().contains("serverConnectionScore"));
}

void TestServerLatency::latency() {
  constexpr int start = 1234;

  // Insert some latency values and see that get/set/avg works as expected.
  ServerLatency serverLatency;
  for (int value = start; value < 5000; value += 100) {
    QString serverPublicKey = "DummyServer" + QString::number(value);

    // Getting the latency for a server that doesn't exist returns zero.
    QCOMPARE(serverLatency.getLatency(serverPublicKey), 0);

    // Set the latency and we should be able to retrive it.
    serverLatency.setLatency(serverPublicKey, value);
    QCOMPARE(serverLatency.getLatency(serverPublicKey), value);

    // We are setting the latencies in an arithmetic sequence, so we have
    // a fairly simple formula for the average too.
    QCOMPARE(serverLatency.avgLatency(), (start + value) / 2);
  }
}

void TestServerLatency::cooldown() {
  ServerLatency serverLatency;

  // The cooldown should be zero until otherwise set.
  QCOMPARE(serverLatency.getCooldown("Some Server"), 0);
  serverLatency.setCooldown("Some Server", 1234);
  QVERIFY(serverLatency.getCooldown("Some Server") >
          QDateTime::currentSecsSinceEpoch());

  // Clear the cooldown and it should re-set to zero.
  serverLatency.setCooldown("Some Server", 0);
  QCOMPARE(serverLatency.getCooldown("Some Server"), 0);
}

constexpr const char* testServerCountryCode = "Middle Earth";

void TestServerLatency::baseCityScore_data() {
  QTest::addColumn<QJsonObject>("json");
  QTest::addColumn<QString>("userCountry");
  QTest::addColumn<QStringList>("serversOnCooldown");
  QTest::addColumn<ServerLatency::ConnectionScores>("score");

  QJsonObject obj;
  QJsonArray servers;
  obj.insert("name", "Mordor");
  obj.insert("code", "mrdr");
  obj.insert("latitude", 3.14159);
  obj.insert("longitude", -2.718);
  obj.insert("servers", servers);

  QTest::addRow("no servers -> unavailable")
      << obj << "Example Country" << QStringList()
      << ServerLatency::Unavailable;

  QJsonObject server;
  server.insert("hostname", "wireguard-1.example.com");
  server.insert("ipv4_addr_in", "169.254.0.1");
  server.insert("ipv4_gateway", "169.254.0.2");
  server.insert("ipv6_addr_in", "fc00:dead:beef::face:cafe");
  server.insert("ipv6_gateway", "fc00:dead:beef::1337:c0de");
  server.insert("public_key", "OnceUponATimeThereWasCakeButIAteIt");
  server.insert("weight", 9000);
  server.insert("multihop_port", 1234);
  server.insert("socks5_name", "socks5.wireguard-1.example.com");
  servers.append(server);
  obj.insert("servers", servers);
  QTest::addRow("one server -> poor")
      << obj << "Example Country" << QStringList() << ServerLatency::Poor;

  QTest::addRow("one server on cooldown -> unavailable")
      << obj << "Example Country"
      << QStringList(server.value("public_key").toString())
      << ServerLatency::Unavailable;

  // Add two more servers to meet the minimum redundancy requirements.
  server.insert("hostname", "wireguard-2.example.com");
  server.insert("public_key", "PieIsBetterAnywaysItsFullOfFruitAndPastry");
  server.insert("socks5_name", "socks5.wireguard-2.example.com");
  servers.append(server);
  obj.insert("servers", servers);
  server.insert("hostname", "wireguard-3.example.com");
  server.insert("public_key", "ThePastryIsWarmAndFlakyAndBakedToPerfection");
  server.insert("socks5_name", "socks5.wireguard-3.example.com");
  servers.append(server);
  obj.insert("servers", servers);
  QTest::addRow("three servers -> moderate")
      << obj << "Example Country" << QStringList() << ServerLatency::Moderate;

  QTest::addRow("three servers one on cooldown -> poor")
      << obj << "Example Country"
      << QStringList(server.value("public_key").toString())
      << ServerLatency::Poor;

  QTest::addRow("same country -> good")
      << obj << QString(testServerCountryCode).toLower() << QStringList()
      << ServerLatency::Good;

  QTest::addRow("case sensitive -> still good")
      << obj << QString(testServerCountryCode).toUpper() << QStringList()
      << ServerLatency::Good;
}

void TestServerLatency::baseCityScore() {
  QFETCH(QJsonObject, json);
  QFETCH(QString, userCountry);
  QFETCH(QStringList, serversOnCooldown);
  QFETCH(ServerLatency::ConnectionScores, score);

  ServerLatency serverLatency;
  for (const QString& pubkey : serversOnCooldown) {
    serverLatency.setCooldown(pubkey,
                              AppConstants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);
  }

  // Establish an average latency of around 100ms.
  for (int i = 0; i < 100; i++) {
    serverLatency.setLatency("DummyServer" + QString::number(i), 100);
  }

  ServerCity city;
  QVERIFY(city.fromJson(json, testServerCountryCode));

  QCOMPARE(serverLatency.baseCityScore(&city, userCountry), score);
}

static TestServerLatency s_testServerLatency;
