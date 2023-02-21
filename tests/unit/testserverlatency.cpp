/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserverlatency.h"

#include "feature.h"
#include "models/servercity.h"
#include "models/location.h"
#include "serverlatency.h"
#include "settingsholder.h"

#include <QDateTime>

void TestServerLatency::init() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"serverConnectionScore"});
  QVERIFY(settingsHolder.featuresFlippedOn().contains("serverConnectionScore"));
}

void TestServerLatency::latency() {
  constexpr int start = 1234;
  int value;

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
  QVERIFY(serverLatency.getCooldown("Some Server") > QDateTime::currentSecsSinceEpoch());

  // Clear the cooldown and it should re-set to zero.
  serverLatency.setCooldown("Some Server", 0);
  QCOMPARE(serverLatency.getCooldown("Some Server"), 0);
}

void TestServerLatency::baseCityScore_data() {
  // TODO: Implement Me!
}

void TestServerLatency::baseCityScore() {
  // TODO: Implement Me!
}

static TestServerLatency s_testServerLatency;
