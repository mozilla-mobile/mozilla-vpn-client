/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmodels.h"
#include "../src/device.h"
#include "../src/devicemodel.h"
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
                          << "" << QDateTime() << ""
                          << "";

    QJsonObject d;
    obj.insert("test", d);
    QTest::addRow("empty") << QJsonDocument(obj).toJson() << false << ""
                           << "" << QDateTime() << ""
                           << "";

    d.insert("name", "deviceName");
    obj.insert("test", d);
    QTest::addRow("name") << QJsonDocument(obj).toJson() << false << ""
                          << "" << QDateTime() << ""
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

// DeviceModel
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::deviceModelBasic()
{
    DeviceModel dm;
    QCOMPARE(dm.hasDevice("foo"), false);
    dm.removeDevice("foo");
    QCOMPARE(dm.device("foo"), nullptr);
    QCOMPARE(dm.activeDevices(), 0);
    QCOMPARE(dm.currentDevice(), nullptr);

    QHash<int, QByteArray> rn = dm.roleNames();
    QCOMPARE(rn.count(), 3);
    QCOMPARE(rn[DeviceModel::NameRole], "name");
    QCOMPARE(rn[DeviceModel::CurrentOneRole], "currentOne");
    QCOMPARE(rn[DeviceModel::CreatedAtRole], "createdAt");

    QCOMPARE(dm.rowCount(QModelIndex()), 0);
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());
}

void TestModels::deviceModelFromJson_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<int>("devices");
    QTest::addColumn<QVariant>("deviceName");
    QTest::addColumn<QVariant>("currentOne");
    QTest::addColumn<QVariant>("createdAt");

    QTest::addRow("invalid") << QByteArray("") << false;
    QTest::addRow("array") << QByteArray("[]") << false;

    QJsonObject obj;
    QTest::addRow("empty") << QJsonDocument(obj).toJson() << false;

    obj.insert("devices", 42);
    QTest::addRow("invalid devices") << QJsonDocument(obj).toJson() << false;

    QJsonArray devices;
    obj.insert("devices", devices);
    QTest::addRow("good but empty")
        << QJsonDocument(obj).toJson() << true << 0 << QVariant() << QVariant() << QVariant();

    QJsonObject d;
    d.insert("name", "deviceName");
    d.insert("pubkey", "devicePubkey");
    d.insert("created_at", "2017-07-24T15:46:29");
    d.insert("ipv4_address", "deviceIpv4");
    d.insert("ipv6_address", "deviceIpv6");

    devices.append(d);
    obj.insert("devices", devices);
    QTest::addRow("good") << QJsonDocument(obj).toJson() << true << 1 << QVariant("deviceName")
                          << QVariant(false)
                          << QVariant(QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate));
}

void TestModels::deviceModelFromJson()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, result);

    DeviceModel dm;
    QCOMPARE(dm.fromJson(json), result);

    if (!result) {
        return;
    }

    QFETCH(int, devices);
    QFETCH(QVariant, deviceName);
    QFETCH(QVariant, currentOne);
    QFETCH(QVariant, createdAt);

    QCOMPARE(dm.rowCount(QModelIndex()), devices);
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::CurrentOneRole), QVariant());
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::CreatedAtRole), QVariant());

    QModelIndex index = dm.index(0, 0);
    QCOMPARE(dm.data(index, DeviceModel::NameRole), deviceName);
    QCOMPARE(dm.data(index, DeviceModel::CurrentOneRole), currentOne);
    QCOMPARE(dm.data(index, DeviceModel::CreatedAtRole), createdAt);
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
