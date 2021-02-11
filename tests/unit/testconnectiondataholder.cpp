/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testconnectiondataholder.h"
#include "../../src/connectiondataholder.h"
#include "../../src/constants.h"
#include "helper.h"

#include <QSplineSeries>
#include <QValueAxis>

void TestConnectionDataHolder::checkIpAddressFailure() {
  ConnectionDataHolder cdh;

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));

  QEventLoop loop;
  connect(&cdh, &ConnectionDataHolder::ipAddressChecked, [&] {
    cdh.disable();
    loop.exit();
  });

  cdh.enable();
  loop.exec();
}

void TestConnectionDataHolder::checkIpAddressSucceess_data() {
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

void TestConnectionDataHolder::checkIpAddressSucceess() {
  ConnectionDataHolder cdh;
  QSignalSpy spy(&cdh, &ConnectionDataHolder::ipAddressChanged);

  QFETCH(QByteArray, json);
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));

  QEventLoop loop;
  connect(&cdh, &ConnectionDataHolder::ipAddressChecked, [&] {
    cdh.disable();

    QFETCH(QString, ipAddress);
    QCOMPARE(cdh.ipAddress(), ipAddress);

    QFETCH(bool, signal);
    QCOMPARE(spy.count(), signal ? 1 : 0);
    loop.exit();
  });

  cdh.enable();
  loop.exec();
}

void TestConnectionDataHolder::chart() {
  ConnectionDataHolder cdh;
  QSignalSpy spy(&cdh, &ConnectionDataHolder::bytesChanged);

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success, QString("{'ip':'42'}").toUtf8()));

  cdh.add(123, 123);
  QCOMPARE(spy.count(), 0);

  QtCharts::QSplineSeries* txSeries = new QtCharts::QSplineSeries(this);
  QtCharts::QSplineSeries* rxSeries = new QtCharts::QSplineSeries(this);
  QtCharts::QValueAxis* axisX = new QtCharts::QValueAxis(this);
  QtCharts::QValueAxis* axisY = new QtCharts::QValueAxis(this);

  cdh.activate(QVariant::fromValue(txSeries), QVariant::fromValue(rxSeries),
               QVariant::fromValue(axisX), QVariant::fromValue(axisY));

  QCOMPARE(spy.count(), 0);
  QCOMPARE(txSeries->count(), Constants::CHARTS_MAX_POINTS);
  QCOMPARE(rxSeries->count(), Constants::CHARTS_MAX_POINTS);

  QEventLoop loop;
  connect(&cdh, &ConnectionDataHolder::bytesChanged, [&] {
    if (spy.count() >= Constants::CHARTS_MAX_POINTS * 2) {
      loop.exit();
    }
  });

  loop.exec();

  QCOMPARE(cdh.txBytes(), (uint32_t)0);
  QCOMPARE(cdh.rxBytes(), (uint32_t)0);
}

static TestConnectionDataHolder s_testConnectionDataHolder;
