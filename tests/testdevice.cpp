/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdevice.h"
#include "../src/device.h"
#include "autotest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestDevice::basic()
{
    Device device("name", QDateTime(), "publicKey", "ipv4Address", "ipv6Address");
    QCOMPARE(device.name(), "name");
    QCOMPARE(device.createdAt(), QDateTime());
    QCOMPARE(device.publicKey(), "publicKey");
    QCOMPARE(device.ipv4Address(), "ipv4Address");
    QCOMPARE(device.ipv6Address(), "ipv6Address");
    QCOMPARE(device.isDevice("name"), true);
    QCOMPARE(device.isDevice("another device"), false);
}

void TestDevice::fromJson_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("publicKey");
    QTest::addColumn<QDateTime>("createdAt");
    QTest::addColumn<QString>("ipv4Address");
    QTest::addColumn<QString>("ipv6Address");

    QJsonObject obj;
    obj.insert("test", "");
    QTest::addRow("null") << QJsonDocument(obj).toJson() << false << ""
                          << ""
                          << QDateTime()
                          << ""
                          << "";

    QJsonObject d;
    obj.insert("test", d);
    QTest::addRow("empty") << QJsonDocument(obj).toJson() << false << ""
                           << ""
                           << QDateTime()
                           << ""
                           << "";

    d.insert("name", "deviceName");
    obj.insert("test", d);
    QTest::addRow("name") << QJsonDocument(obj).toJson() << false << ""
                          << ""
                          << QDateTime()
                          << ""
                          << "";

    d.insert("pubkey", "devicePubkey");
    obj.insert("test", d);
    QTest::addRow("pubKey") << QJsonDocument(obj).toJson() << false << ""
                            << "" << QDateTime() << ""
                            << "";

    d.insert("created_at", "2017-07-24T15:46:29");
    obj.insert("test", d);
    QTest::addRow("createdAt") << QJsonDocument(obj).toJson() << false << ""
                               << "" << QDateTime() << ""
                               << "";

    d.insert("ipv4_address", "deviceIpv4");
    obj.insert("test", d);
    QTest::addRow("ipv4Address") << QJsonDocument(obj).toJson() << false << ""
                                 << "" << QDateTime() << ""
                                 << "";

    d.insert("ipv6_address", "deviceIpv6");
    obj.insert("test", d);
    QTest::addRow("ipv6Address") << QJsonDocument(obj).toJson() << true << "deviceName"
                                 << "devicePubkey"
                                 << QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate)
                                 << "deviceIpv4"
                                 << "deviceIpv6";
}

void TestDevice::fromJson()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, result);
    QFETCH(QString, name);
    QFETCH(QString, publicKey);
    QFETCH(QDateTime, createdAt);
    QFETCH(QString, ipv4Address);
    QFETCH(QString, ipv6Address);

    QJsonDocument doc = QJsonDocument::fromJson(json);
    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();
    Q_ASSERT(obj.contains("test"));

    Device device;
    QCOMPARE(device.fromJson(obj.take("test")), result);
    QCOMPARE(device.name(), name);
    QCOMPARE(device.publicKey(), publicKey);
    QCOMPARE(device.createdAt(), createdAt);
    QCOMPARE(device.ipv4Address(), ipv4Address);
    QCOMPARE(device.ipv6Address(), ipv6Address);
}

DECLARE_TEST(TestDevice)
