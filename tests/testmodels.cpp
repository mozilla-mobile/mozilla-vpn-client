/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmodels.h"
#include "../src/device.h"
#include "../src/keys.h"
#include "../src/servercity.h"
#include "../src/servercountry.h"
#include "../src/serverdata.h"
#include "../src/user.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// Device
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::deviceBasic()
{
    Device device;
    QCOMPARE(device.name(), "");
    QCOMPARE(device.createdAt(), QDateTime());
    QCOMPARE(device.publicKey(), "");
    QCOMPARE(device.ipv4Address(), "");
    QCOMPARE(device.ipv6Address(), "");
    QCOMPARE(device.isDevice("name"), false);
}

void TestModels::deviceFromJson_data()
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

void TestModels::deviceFromJson()
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

// Keys
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::keysBasic()
{
    Keys k;
    QCOMPARE(k.privateKey(), "");

    k.storeKey("test");
    QCOMPARE(k.privateKey(), "test");

    k.forgetKey();
    QCOMPARE(k.privateKey(), "");
}

// ServerData
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverDataBasic()
{
    ServerData sd;

    QCOMPARE(sd.initialized(), false);
    QCOMPARE(sd.countryCode(), "");
    QCOMPARE(sd.city(), "");

    {
        QJsonObject countryObj;
        countryObj.insert("name", "serverCountryName");
        countryObj.insert("code", "serverCountryCode");
        countryObj.insert("cities", QJsonArray());
        ServerCountry country = ServerCountry::fromJson(countryObj);

        QJsonObject cityObj;
        cityObj.insert("code", "serverCityCode");
        cityObj.insert("name", "serverCityName");
        cityObj.insert("servers", QJsonArray());
        ServerCity city = ServerCity::fromJson(cityObj);

        sd.initialize(country, city);
        QCOMPARE(sd.initialized(), true);
        QCOMPARE(sd.countryCode(), "serverCountryCode");
        QCOMPARE(sd.city(), "serverCityName");
    }

    sd.update("new Country", "new City");
    QCOMPARE(sd.initialized(), true);
    QCOMPARE(sd.countryCode(), "new Country");
    QCOMPARE(sd.city(), "new City");

    sd.forget();
    QCOMPARE(sd.initialized(), false);
    QCOMPARE(sd.countryCode(), "new Country");
    QCOMPARE(sd.city(), "new City");
}
// User
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::userBasic()
{
    User user;
    QCOMPARE(user.avatar(), "");
    QCOMPARE(user.displayName(), "");
    QCOMPARE(user.email(), "");
    QCOMPARE(user.maxDevices(), 5);
    QCOMPARE(user.subscriptionNeeded(), false);
}

void TestModels::userFromJson_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("avatar");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("email");
    QTest::addColumn<int>("maxDevices");
    QTest::addColumn<bool>("subscriptionNeeded");

    QTest::newRow("null") << QByteArray("") << false << ""
                          << ""
                          << "" << 5 << false;
    QTest::newRow("invalid") << QByteArray("wow") << false << ""
                             << ""
                             << "" << 5 << false;
    QTest::newRow("array") << QByteArray("[]") << false << ""
                           << ""
                           << "" << 5 << false;

    QJsonObject obj;

    QTest::newRow("empty object") << QJsonDocument(obj).toJson() << false << ""
                                  << ""
                                  << "" << 5 << false;

    obj.insert("a", QJsonValue("b"));
    QTest::newRow("no avatar") << QJsonDocument(obj).toJson() << false << ""
                               << ""
                               << "" << 5 << false;

    obj.insert("avatar", QJsonValue("avatar"));
    QTest::newRow("no displayName") << QJsonDocument(obj).toJson() << false << "avatar"
                                    << ""
                                    << "" << 5 << false;

    obj.insert("display_name", QJsonValue("displayName"));
    QTest::newRow("no email") << QJsonDocument(obj).toJson() << false << "avatar"
                              << "displayName"
                              << "" << 5 << false;

    obj.insert("email", QJsonValue("email"));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false << "avatar"
                                   << "displayName"
                                   << "email" << 5 << false;

    obj.insert("max_devices", QJsonValue(123));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false << "avatar"
                                   << "displayName"
                                   << "email" << 123 << false;

    obj.insert("subscriptions", QJsonValue("wow"));
    QTest::newRow("invalid subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                          << "displayName"
                                          << "email" << 123 << false;

    QJsonObject subscription;
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                        << "displayName"
                                        << "email" << 123 << true;

    subscription.insert("vpn", QJsonValue("WOW"));
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid vpn subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                              << "displayName"
                                              << "email" << 123 << true;

    QJsonObject subVpn;
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty vpn subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                            << "displayName"
                                            << "email" << 123 << true;

    subVpn.insert("active", QJsonValue("sure!"));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid active vpn subscription")
        << QJsonDocument(obj).toJson() << false << "avatar"
        << "displayName"
        << "email" << 123 << true;

    subVpn.insert("active", QJsonValue(true));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("active vpn subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                             << "displayName"
                                             << "email" << 123 << false;

    subVpn.insert("active", QJsonValue(false));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("inactive vpn subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                               << "displayName"
                                               << "email" << 123 << true;
}

void TestModels::userFromJson()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, result);
    QFETCH(QString, avatar);
    QFETCH(QString, displayName);
    QFETCH(QString, email);
    QFETCH(int, maxDevices);
    QFETCH(bool, subscriptionNeeded);

    User user;
    QCOMPARE(user.fromJson(json), result);

    QCOMPARE(user.avatar(), avatar);
    QCOMPARE(user.displayName(), displayName);
    QCOMPARE(user.email(), email);
    QCOMPARE(user.maxDevices(), maxDevices);
    QCOMPARE(user.subscriptionNeeded(), subscriptionNeeded);
}

QTEST_MAIN(TestModels)
