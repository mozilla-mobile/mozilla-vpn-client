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
    QJsonDocument doc = QJsonDocument::fromJson(json);

    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();
    Q_ASSERT(obj.contains("test"));

    Device device;

    QFETCH(bool, result);
    QCOMPARE(device.fromJson(obj.take("test")), result);

    QFETCH(QString, name);
    QCOMPARE(device.name(), name);

    QFETCH(QString, publicKey);
    QCOMPARE(device.publicKey(), publicKey);

    QFETCH(QDateTime, createdAt);
    QCOMPARE(device.createdAt(), createdAt);

    QFETCH(QString, ipv4Address);
    QCOMPARE(device.ipv4Address(), ipv4Address);

    QFETCH(QString, ipv6Address);
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

    QCOMPARE(dm.rowCount(QModelIndex()), devices);
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::CurrentOneRole), QVariant());
    QCOMPARE(dm.data(QModelIndex(), DeviceModel::CreatedAtRole), QVariant());

    QModelIndex index = dm.index(0, 0);

    QFETCH(QVariant, deviceName);
    QCOMPARE(dm.data(index, DeviceModel::NameRole), deviceName);

    QFETCH(QVariant, currentOne);
    QCOMPARE(dm.data(index, DeviceModel::CurrentOneRole), currentOne);

    QFETCH(QVariant, createdAt);
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

// Server
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverBasic()
{
    Server s;
    QCOMPARE(s.initialized(), false);
    QCOMPARE(s.hostname(), "");
    QCOMPARE(s.ipv4AddrIn(), "");
    QCOMPARE(s.ipv4Gateway(), "");
    QCOMPARE(s.ipv6AddrIn(), "");
    QCOMPARE(s.ipv6Gateway(), "");
    QCOMPARE(s.publicKey(), "");
    QCOMPARE(s.weight(), (uint32_t)0);
    QCOMPARE(s.choosePort(), (uint32_t)0);
}

void TestModels::serverFromJson_data()
{
    QTest::addColumn<QJsonObject>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("hostname");
    QTest::addColumn<QString>("ipv4AddrIn");
    QTest::addColumn<QString>("ipv4Gateway");
    QTest::addColumn<QString>("ipv6AddrIn");
    QTest::addColumn<QString>("ipv6Gateway");
    QTest::addColumn<QString>("publicKey");
    QTest::addColumn<int>("weight");
    QTest::addColumn<int>("port");

    QJsonObject obj;
    QTest::addRow("empty") << obj << false;

    obj.insert("hostname", "hostname");
    QTest::addRow("hostname") << obj << false;

    obj.insert("ipv4_addr_in", "ipv4AddrIn");
    QTest::addRow("ipv4AddrIn") << obj << false;

    obj.insert("ipv4_gateway", "ipv4Gateway");
    QTest::addRow("ipv4Gateway") << obj << false;

    obj.insert("ipv6_addr_in", "ipv6AddrIn");
    QTest::addRow("ipv6AddrIn") << obj << false;

    obj.insert("ipv6_gateway", "ipv6Gateway");
    QTest::addRow("ipv6Gateway") << obj << false;

    obj.insert("public_key", "publicKey");
    QTest::addRow("publicKey") << obj << false;

    obj.insert("weight", 1234);
    QTest::addRow("weight") << obj << false;

    QJsonArray portRanges;
    obj.insert("port_ranges", portRanges);
    QTest::addRow("portRanges") << obj << true << "hostname"
                                << "ipv4AddrIn"
                                << "ipv4Gateway"
                                << "ipv6AddrIn"
                                << "ipv6Gateway"
                                << "publicKey" << 1234 << 0;

    portRanges.append(42);
    obj.insert("port_ranges", portRanges);
    QTest::addRow("portRanges wrong type") << obj << false;

    QJsonArray portRange;
    portRanges.replace(0, portRange);
    obj.insert("port_ranges", portRanges);
    QTest::addRow("portRanges wrong number") << obj << false;

    portRange.append("A");
    portRange.append("B");
    portRanges.replace(0, portRange);
    obj.insert("port_ranges", portRanges);
    QTest::addRow("portRanges wrong type") << obj << false;

    portRange.replace(0, 42);
    portRange.replace(1, 42);
    portRanges.replace(0, portRange);
    obj.insert("port_ranges", portRanges);
    QTest::addRow("all good") << obj << true << "hostname"
                              << "ipv4AddrIn"
                              << "ipv4Gateway"
                              << "ipv6AddrIn"
                              << "ipv6Gateway"
                              << "publicKey" << 1234 << 42;
}

void TestModels::serverFromJson()
{
    QFETCH(QJsonObject, json);
    QFETCH(bool, result);

    Server s;
    QCOMPARE(s.fromJson(json), result);

    if (!result) {
        QCOMPARE(s.initialized(), false);
        return;
    }

    QCOMPARE(s.initialized(), true);

    QFETCH(QString, hostname);
    QCOMPARE(s.hostname(), hostname);

    QFETCH(QString, ipv4AddrIn);
    QCOMPARE(s.ipv4AddrIn(), ipv4AddrIn);

    QFETCH(QString, ipv4Gateway);
    QCOMPARE(s.ipv4Gateway(), ipv4Gateway);

    QFETCH(QString, ipv6AddrIn);
    QCOMPARE(s.ipv6AddrIn(), ipv6AddrIn);

    QFETCH(QString, ipv6Gateway);
    QCOMPARE(s.ipv6Gateway(), ipv6Gateway);

    QFETCH(QString, publicKey);
    QCOMPARE(s.publicKey(), publicKey);

    QFETCH(int, weight);
    QCOMPARE(s.weight(), (uint32_t)weight);

    QFETCH(int, port);
    QCOMPARE(s.choosePort(), (uint32_t)port);
}

// ServerCity
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverCityBasic()
{
    ServerCity sc;
    QCOMPARE(sc.name(), "");
    QCOMPARE(sc.code(), "");
    QCOMPARE(sc.getServers().isEmpty(), true);
}

void TestModels::serverCityFromJson_data()
{
    QTest::addColumn<QJsonObject>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("code");
    QTest::addColumn<int>("servers");

    QJsonObject obj;
    QTest::addRow("empty") << obj << false;

    obj.insert("name", "name");
    QTest::addRow("name") << obj << false;

    obj.insert("code", "code");
    QTest::addRow("code") << obj << false;

    obj.insert("servers", "servers");
    QTest::addRow("servers invalid") << obj << false;

    QJsonArray servers;
    obj.insert("servers", servers);
    QTest::addRow("servers empty") << obj << true << "name" << "code" << 0;
}

void TestModels::serverCityFromJson()
{
    QFETCH(QJsonObject, json);
    QFETCH(bool, result);

    ServerCity sc;
    QCOMPARE(sc.fromJson(json), result);
    if (!result) {
        QCOMPARE(sc.name(), "");
        QCOMPARE(sc.code(), "");
        QCOMPARE(sc.getServers().isEmpty(), true);
        return;
    }

    QFETCH(QString, name);
    QCOMPARE(sc.name(), name);

    QFETCH(QString, code);
    QCOMPARE(sc.code(), code);

    QFETCH(int, servers);
    QCOMPARE(sc.getServers().length(), servers);
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

        ServerCity city;
        QCOMPARE(city.fromJson(cityObj), true);

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

    QTest::newRow("null") << QByteArray("") << false;
    QTest::newRow("invalid") << QByteArray("wow") << false;
    QTest::newRow("array") << QByteArray("[]") << false;

    QJsonObject obj;

    QTest::newRow("empty object") << QJsonDocument(obj).toJson() << false;

    obj.insert("a", QJsonValue("b"));
    QTest::newRow("no avatar") << QJsonDocument(obj).toJson() << false;

    obj.insert("avatar", QJsonValue("avatar"));
    QTest::newRow("no displayName") << QJsonDocument(obj).toJson() << false;

    obj.insert("display_name", QJsonValue("displayName"));
    QTest::newRow("no email") << QJsonDocument(obj).toJson() << false;

    obj.insert("email", QJsonValue("email"));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false;

    obj.insert("max_devices", QJsonValue(123));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false;

    obj.insert("subscriptions", QJsonValue("wow"));
    QTest::newRow("invalid subscription") << QJsonDocument(obj).toJson() << false;

    QJsonObject subscription;
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                        << "displayName"
                                        << "email" << 123 << true;

    subscription.insert("vpn", QJsonValue("WOW"));
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid vpn subscription") << QJsonDocument(obj).toJson() << false;

    QJsonObject subVpn;
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty vpn subscription") << QJsonDocument(obj).toJson() << false;

    subVpn.insert("active", QJsonValue("sure!"));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid active vpn subscription") << QJsonDocument(obj).toJson() << false;

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

    User user;
    QCOMPARE(user.fromJson(json), result);

    if (!result) {
        QCOMPARE(user.avatar(), "");
        QCOMPARE(user.displayName(), "");
        QCOMPARE(user.email(), "");
        QCOMPARE(user.maxDevices(), 5);
        QCOMPARE(user.subscriptionNeeded(), false);
        return;
    }

    QFETCH(QString, avatar);
    QCOMPARE(user.avatar(), avatar);

    QFETCH(QString, displayName);
    QCOMPARE(user.displayName(), displayName);

    QFETCH(QString, email);
    QCOMPARE(user.email(), email);

    QFETCH(int, maxDevices);
    QCOMPARE(user.maxDevices(), maxDevices);

    QFETCH(bool, subscriptionNeeded);
    QCOMPARE(user.subscriptionNeeded(), subscriptionNeeded);
}

QTEST_MAIN(TestModels)
