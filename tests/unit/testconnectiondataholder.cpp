/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testconnectiondataholder.h"
#include "../../src/connectiondataholder.h"
#include "../../src/constants.h"
#include "../../src/settingsholder.h"
#include "helper.h"

#include <QSplineSeries>
#include <QValueAxis>

#if QT_VERSION < 0x060000
using namespace QtCharts;
#endif

void TestConnectionDataHolder::checkIpAddressFailure() {
  ConnectionDataHolder cdh;

  SettingsHolder settingsHolder;

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));
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
  QSignalSpy spy(&cdh, &ConnectionDataHolder::ipv4AddressChanged);

  SettingsHolder settingsHolder;

  TestHelper::networkConfig.clear();

  QFETCH(QByteArray, json);
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));

  QEventLoop loop;
  connect(&cdh, &ConnectionDataHolder::ipAddressChecked, [&] {
    cdh.disable();

    QFETCH(QString, ipAddress);
    QCOMPARE(cdh.ipv4Address(), ipAddress);

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

  SettingsHolder settingsHolder;

  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success, QString("{'ip':'42'}").toUtf8()));
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success, QString("{'ip':'42'}").toUtf8()));

  cdh.add(123, 123);
  QCOMPARE(spy.count(), 0);

  QSplineSeries* txSeries = new QSplineSeries(this);
  QSplineSeries* rxSeries = new QSplineSeries(this);
  QValueAxis* axisX = new QValueAxis(this);
  QValueAxis* axisY = new QValueAxis(this);

  cdh.activate(QVariant::fromValue(txSeries), QVariant::fromValue(rxSeries),
               QVariant::fromValue(axisX), QVariant::fromValue(axisY));

  QCOMPARE(spy.count(), 0);
  QCOMPARE(txSeries->count(), Constants::chartsMaxPoints());
  QCOMPARE(rxSeries->count(), Constants::chartsMaxPoints());

  QEventLoop loop;
  connect(&cdh, &ConnectionDataHolder::bytesChanged, [&] {
    if (spy.count() >= Constants::chartsMaxPoints() * 2) {
      loop.exit();
    }
  });

  loop.exec();

  QCOMPARE(cdh.txBytes(), (uint32_t)0);
  QCOMPARE(cdh.rxBytes(), (uint32_t)0);
}

static TestConnectionDataHolder s_testConnectionDataHolder;
