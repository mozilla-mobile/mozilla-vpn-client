/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testreleasemonitor.h"
#include "../../src/releasemonitor.h"
#include "../../src/update/versionapi.h"
#include "helper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestReleaseMonitor::failure() {
  qDebug() << "SET";
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));

  ReleaseMonitor rm;
  rm.runSoon();

  QEventLoop loop;
  connect(&rm, &ReleaseMonitor::releaseChecked, &rm, [&] { loop.exit(); });
  loop.exec();
}

void TestReleaseMonitor::success_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << QByteArray("") << false;

  QJsonObject obj;
  QTest::addRow("empty object") << QJsonDocument(obj).toJson() << false;

  QJsonValue value(42);
  obj.insert("linux", value);
  obj.insert("ios", value);
  obj.insert("macos", value);
  obj.insert("dummy", value);
  QTest::addRow("invalid platform") << QJsonDocument(obj).toJson() << false;

  QJsonObject platform;
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("empty platform") << QJsonDocument(obj).toJson() << false;

  QJsonObject latest;
  platform.insert("latest", latest);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("empty latest") << QJsonDocument(obj).toJson() << false;

  latest.insert("version", 42);
  platform.insert("latest", latest);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("invalid latest version")
      << QJsonDocument(obj).toJson() << false;

  latest.insert("version", "42");
  platform.insert("latest", latest);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("missing minimum") << QJsonDocument(obj).toJson() << false;

  QJsonObject minimum;
  minimum.insert("version", 42);
  platform.insert("minimum", minimum);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("invalid minimum version")
      << QJsonDocument(obj).toJson() << false;

  minimum.insert("version", "42");
  platform.insert("minimum", minimum);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("all good") << QJsonDocument(obj).toJson() << true;

  minimum.insert("version", "9999");
  platform.insert("minimum", minimum);
  obj.insert("linux", platform);
  obj.insert("ios", platform);
  obj.insert("macos", platform);
  obj.insert("dummy", platform);
  QTest::addRow("completed!") << QJsonDocument(obj).toJson() << true;
}

void TestReleaseMonitor::success() {
  ReleaseMonitor rm;
  rm.runSoon();

  QFETCH(QByteArray, json);
  TestHelper::networkConfig.append(
      TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));

  QEventLoop loop;
  connect(&rm, &ReleaseMonitor::releaseChecked, &rm, [&] { loop.exit(); });
  loop.exec();
}

void TestReleaseMonitor::compareVersions_data() {
  QTest::addColumn<QString>("a");
  QTest::addColumn<QString>("b");
  QTest::addColumn<int>("result");

  QTest::addRow("empty a") << ""
                           << "123" << 1;
  QTest::addRow("empty b") << "123"
                           << "" << -1;
  QTest::addRow("empty all") << ""
                             << "" << 0;

  QTest::addRow("equal 1") << "0.1"
                           << "0.1" << 0;
  QTest::addRow("equal 2") << "0.1.2"
                           << "0.1.2" << 0;
  QTest::addRow("equal 3") << "0.1.2.3"
                           << "0.1.2.3" << 0;
  QTest::addRow("equal 4") << "0"
                           << "0" << 0;
  QTest::addRow("equal 5") << "123"
                           << "123" << 0;
  QTest::addRow("euqal 6") << "0.1.2.123"
                           << "0.1.2.456" << 0;

  QTest::addRow("a wins 1") << "0"
                            << "123" << -1;
  QTest::addRow("a wins 2") << "0.1"
                            << "123" << -1;
  QTest::addRow("a wins 3") << "0.1.2"
                            << "123" << -1;
  QTest::addRow("a wins 4") << "0.1.2.3"
                            << "123" << -1;
  QTest::addRow("a wins 5") << "0.1.2.3.4"
                            << "123" << -1;
  QTest::addRow("a wins 6") << "1.2.3.4"
                            << "123" << -1;
  QTest::addRow("a wins 7") << "0"
                            << "1" << -1;
  QTest::addRow("a wins 8") << "0"
                            << "0.1" << -1;
  QTest::addRow("a wins 9") << "0"
                            << "0.1.2" << -1;
  QTest::addRow("a wins A") << "0"
                            << "0.1.2.3" << -1;
  QTest::addRow("a wins B") << "0.1"
                            << "1" << -1;
  QTest::addRow("a wins C") << "0.1"
                            << "0.2" << -1;
  QTest::addRow("a wins D") << "0.1"
                            << "0.1.2" << -1;
  QTest::addRow("a wins E") << "0.1.2"
                            << "1" << -1;
  QTest::addRow("a wins F") << "0.1.2"
                            << "0.2" << -1;
  QTest::addRow("a wins 10") << "0.1.2"
                             << "0.1.3" << -1;

  QTest::addRow("b wins 1") << "123"
                            << "0" << 1;
  QTest::addRow("b wins 2") << "123"
                            << "0.1" << 1;
  QTest::addRow("b wins 3") << "123"
                            << "0.1.2" << 1;
  QTest::addRow("b wins 4") << "123"
                            << "0.1.2.3" << 1;
  QTest::addRow("b wins 5") << "123"
                            << "0.1.2.3.4" << 1;
  QTest::addRow("b wins 6") << "123"
                            << "1.2.3.4" << 1;
  QTest::addRow("b wins 7") << "1"
                            << "0" << 1;
  QTest::addRow("b wins 8") << "0.1"
                            << "0" << 1;
  QTest::addRow("b wins 9") << "0.1.2"
                            << "0" << 1;
  QTest::addRow("b wins A") << "0.1.2.3"
                            << "0" << 1;
  QTest::addRow("b wins B") << "1"
                            << "0.1" << 1;
  QTest::addRow("b wins C") << "0.2"
                            << "0.1" << 1;
  QTest::addRow("b wins D") << "0.1.2"
                            << "0.1" << 1;
  QTest::addRow("b wins E") << "1"
                            << "0.1.2" << 1;
  QTest::addRow("b wins F") << "0.2"
                            << "0.1.2" << 1;
  QTest::addRow("b wins 10") << "0.1.3"
                             << "0.1.2" << 1;

  QTest::addRow("a extra") << "1.2.3~4.5.6"
                           << "1.2.3" << 0;
  QTest::addRow("b extra") << "1.2.3"
                           << "1.2.3~4.5.6" << 0;
}

void TestReleaseMonitor::compareVersions() {
  QFETCH(QString, a);
  QFETCH(QString, b);
  QFETCH(int, result);

  QCOMPARE(VersionApi::compareVersions(a, b), result);
}

void TestReleaseMonitor::stripMinor_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("result");

  QTest::addRow("empty") << ""
                         << "0.0.0";
  QTest::addRow("short") << "1"
                         << "1.0.0";
  QTest::addRow("medium") << "1.2"
                          << "1.2.0";
  QTest::addRow("normal") << "1.2.3"
                          << "1.2.0";
  QTest::addRow("big") << "1.2.3.4"
                       << "1.2.0";
  QTest::addRow("extra") << "1.2.3~alpha"
                         << "1.2.0";
}

void TestReleaseMonitor::stripMinor() {
  QFETCH(QString, input);
  QFETCH(QString, result);
  QCOMPARE(VersionApi::stripMinor(input), result);
}

static TestReleaseMonitor s_testReleaseMonitor;
