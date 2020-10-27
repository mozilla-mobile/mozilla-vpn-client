/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testreleasemonitor.h"
#include "../src/releasemonitor.h"
#include "helper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestReleaseMonitor::basic_data()
{
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
    QTest::addRow("invalid latest version") << QJsonDocument(obj).toJson() << false;

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
    QTest::addRow("invalid minimum version") << QJsonDocument(obj).toJson() << false;

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

void TestReleaseMonitor::basic()
{
    ReleaseMonitor rm;

    QFETCH(QByteArray, json);
    QFETCH(bool, result);

    QCOMPARE(rm.processData(json), result);
}

static TestReleaseMonitor s_testReleaseMonitor;
