/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testconnectionhealth.h"

#include "connectionhealth.h"

void TestConnectionHealth::dnsPingReceived() {
  ConnectionHealth connectionHealth;
  connectionHealth.startIdle();
  QVERIFY(!connectionHealth.m_dnsPingInitialized);

  // Ping for invalid sequence number should not be accepted
  connectionHealth.dnsPingReceived(42);
  QVERIFY(!connectionHealth.m_dnsPingInitialized);

  // Ping for matching sequence number should be accepted
  connectionHealth.dnsPingReceived(connectionHealth.m_dnsPingSequence);
  QVERIFY(connectionHealth.m_dnsPingInitialized);
}

void TestConnectionHealth::updateDnsPingLatency() {
  ConnectionHealth connectionHealth;
  std::vector<int> pings({100, 110, 120, 130, 140, 150, 160, 170, 180, 190});
  float weight = 1.0 / PING_BASELINE_EWMA_DIVISOR;

  // Verify latency measurement for each ping by calculating the
  // Exponentially Weighted Moving Average.
  for (int i = 0; i < pings.size(); i++) {
    connectionHealth.updateDnsPingLatency(pings[i]);
    int expectedLatency =
        ewma(std::vector<int>(pings.begin(), pings.begin() + i + 1), weight);
    QCOMPARE(connectionHealth.m_dnsPingLatency, expectedLatency);
  }
}

void TestConnectionHealth::healthCheckup() {
  ConnectionHealth connectionHealth;

  // Signal timer is not active -> NoSignal
  connectionHealth.healthCheckup();
  QCOMPARE(connectionHealth.m_stability,
           ConnectionHealth::ConnectionStability::NoSignal);

  // Signal timer is active, but recent pings were lost -> Unstable
  connectionHealth.startIdle();
  connectionHealth.m_noSignalTimer.start();
  for (int i = 0; i < connectionHealth.m_pingHelper.m_pingData.size(); i++) {
    connectionHealth.m_pingHelper.m_pingData[i].timestamp =
        QDateTime::currentMSecsSinceEpoch() - (60 * 1000);
  }
  connectionHealth.healthCheckup();
  QCOMPARE(connectionHealth.m_stability,
           ConnectionHealth::ConnectionStability::Unstable);

  // Signal timer is active, recent pings not lost -> Stable
  for (int i = 0; i < connectionHealth.m_pingHelper.m_pingData.size(); i++) {
    connectionHealth.m_pingHelper.m_pingData[i].timestamp =
        QDateTime::currentMSecsSinceEpoch();
  }
  connectionHealth.healthCheckup();
  QCOMPARE(connectionHealth.m_stability,
           ConnectionHealth::ConnectionStability::Stable);

  // Signal timer is active, recent ping(s) took too long -> Unstable
  connectionHealth.dnsPingReceived(connectionHealth.m_dnsPingSequence);
  connectionHealth.m_pingHelper.m_pingData[0].latency = INT_MAX;
  connectionHealth.healthCheckup();
  QCOMPARE(connectionHealth.m_stability,
           ConnectionHealth::ConnectionStability::Unstable);

  // Signal timer is active, recent ping(s) arrived on time -> Back to Stable
  connectionHealth.dnsPingReceived(connectionHealth.m_dnsPingSequence);
  connectionHealth.m_pingHelper.m_pingData[0].latency = 0;
  connectionHealth.healthCheckup();
  QCOMPARE(connectionHealth.m_stability,
           ConnectionHealth::ConnectionStability::Stable);
}

static TestConnectionHealth s_testConnectionHealth;