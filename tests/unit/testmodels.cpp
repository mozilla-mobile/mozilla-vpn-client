/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmodels.h"
#include "../../src/ipaddressrange.h"
#include "../../src/models/device.h"
#include "../../src/models/devicemodel.h"
#include "../../src/models/keys.h"
#include "../../src/models/servercity.h"
#include "../../src/models/servercountry.h"
#include "../../src/models/servercountrymodel.h"
#include "../../src/models/serverdata.h"
#include "../../src/models/user.h"
#include "../../src/settingsholder.h"
#include "helper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// Device
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::deviceBasic() {
  Device device;
  QCOMPARE(device.name(), "");
  QCOMPARE(device.createdAt(), QDateTime());
  QCOMPARE(device.publicKey(), "");
  QCOMPARE(device.ipv4Address(), "");
  QCOMPARE(device.ipv6Address(), "");
  QVERIFY(!device.isDevice("name"));
}

void TestModels::deviceCurrentDeviceName() {
  QVERIFY(!Device::currentDeviceName().isEmpty());
}

void TestModels::deviceFromJson_data() {
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

  d.insert("created_at", 42);
  obj.insert("test", d);
  QTest::addRow("createdAt (invalid)")
      << QJsonDocument(obj).toJson() << false << ""
      << "" << QDateTime() << ""
      << "";

  d.insert("created_at", "42");
  obj.insert("test", d);
  QTest::addRow("createdAt (invalid string)")
      << QJsonDocument(obj).toJson() << false << ""
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
  QTest::addRow("ipv6Address")
      << QJsonDocument(obj).toJson() << true << "deviceName"
      << "devicePubkey"
      << QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate)
      << "deviceIpv4"
      << "deviceIpv6";
}

void TestModels::deviceFromJson() {
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

  Device deviceB(device);
  QCOMPARE(deviceB.name(), device.name());
  QCOMPARE(deviceB.createdAt(), device.createdAt());
  QCOMPARE(deviceB.publicKey(), device.publicKey());
  QCOMPARE(deviceB.ipv4Address(), device.ipv4Address());
  QCOMPARE(deviceB.ipv6Address(), device.ipv6Address());

  Device deviceC;
  deviceC = device;
  QCOMPARE(deviceC.name(), device.name());
  QCOMPARE(deviceC.createdAt(), device.createdAt());
  QCOMPARE(deviceC.publicKey(), device.publicKey());
  QCOMPARE(deviceC.ipv4Address(), device.ipv4Address());
  QCOMPARE(deviceC.ipv6Address(), device.ipv6Address());

  device = device;
}

// DeviceModel
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::deviceModelBasic() {
  DeviceModel dm;
  QVERIFY(!dm.initialized());
  QVERIFY(!dm.hasDevice("foo"));
  dm.removeDevice("foo");
  QCOMPARE(dm.device("foo"), nullptr);
  QCOMPARE(dm.activeDevices(), 0);

  Keys keys;
  QCOMPARE(dm.currentDevice(&keys), nullptr);

  QHash<int, QByteArray> rn = dm.roleNames();
  QCOMPARE(rn.count(), 3);
  QCOMPARE(rn[DeviceModel::NameRole], "name");
  QCOMPARE(rn[DeviceModel::CurrentOneRole], "currentOne");
  QCOMPARE(rn[DeviceModel::CreatedAtRole], "createdAt");

  QCOMPARE(dm.rowCount(QModelIndex()), 0);
  QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());

  SettingsHolder settingsHolder;

  QVERIFY(!dm.fromSettings(&keys));

  dm.writeSettings();
  QVERIFY(!dm.fromSettings(&keys));
}

void TestModels::deviceModelFromJson_data() {
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
  QTest::addRow("good but empty") << QJsonDocument(obj).toJson() << true << 0
                                  << QVariant() << QVariant() << QVariant();

  devices.append(42);
  obj.insert("devices", devices);
  QTest::addRow("invalid devices") << QJsonDocument(obj).toJson() << false;

  QJsonObject d;
  d.insert("name", "deviceName");
  d.insert("pubkey", "devicePubkey");
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.replace(0, d);
  obj.insert("devices", devices);
  QTest::addRow("good") << QJsonDocument(obj).toJson() << true << 1
                        << QVariant("deviceName") << QVariant(false)
                        << QVariant(QDateTime::fromString("2017-07-24T15:46:29",
                                                          Qt::ISODate));

  d.insert("name", Device::currentDeviceName());
  d.insert("pubkey", "currentDevicePubkey");
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.append(d);
  obj.insert("devices", devices);
  QTest::addRow("good - 2 devices")
      << QJsonDocument(obj).toJson() << true << 2
      << QVariant(Device::currentDeviceName()) << QVariant(true)
      << QVariant(QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate));
}

void TestModels::deviceModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // fromJson
  {
    Keys keys;
    keys.storeKeys("private", "currentDevicePubkey");

    DeviceModel dm;

    QSignalSpy signalSpy(&dm, &DeviceModel::changed);
    QCOMPARE(dm.fromJson(&keys, json), result);

    if (!result) {
      QVERIFY(!dm.initialized());
      QCOMPARE(signalSpy.count(), 0);
      QCOMPARE(dm.rowCount(QModelIndex()), 0);
    } else {
      QVERIFY(dm.initialized());
      QCOMPARE(signalSpy.count(), 1);

      QFETCH(int, devices);
      QCOMPARE(dm.rowCount(QModelIndex()), devices);
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::CurrentOneRole), QVariant());
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::CreatedAtRole), QVariant());

      QModelIndex index = dm.index(0, 0);

      QFETCH(QVariant, deviceName);
      QCOMPARE(dm.data(index, DeviceModel::NameRole), deviceName);

      // We cannot compare the currentOne with the DM because the Keys object
      // doesn't exist in the MozillaVPN mock object
      // QFETCH(QVariant, currentOne);
      // QCOMPARE(dm.data(index, DeviceModel::CurrentOneRole), currentOne);
      // QCOMPARE(!!dm.currentDevice(&keys), currentOne.toBool());

      QFETCH(QVariant, createdAt);
      QCOMPARE(dm.data(index, DeviceModel::CreatedAtRole), createdAt);

      QCOMPARE(dm.data(index, DeviceModel::CreatedAtRole + 1), QVariant());

      QCOMPARE(dm.activeDevices(), devices);

      if (devices > 0) {
        QVERIFY(dm.hasDevice(deviceName.toString()));
        QVERIFY(dm.device(deviceName.toString()) != nullptr);

        dm.removeDevice("FOO");
        QCOMPARE(dm.activeDevices(), devices);

        dm.removeDevice(deviceName.toString());
        QCOMPARE(dm.activeDevices(), devices - 1);
      }

      QVERIFY(dm.fromJson(&keys, json));
    }
  }

  // fromSettings
  {
    SettingsHolder settingsHolder;
    SettingsHolder::instance()->setDevices(json);

    Keys keys;
    keys.storeKeys("private", "currentDevicePubkey");

    DeviceModel dm;
    QSignalSpy signalSpy(&dm, &DeviceModel::changed);
    QCOMPARE(dm.fromSettings(&keys), result);

    if (!result) {
      QVERIFY(!dm.initialized());
      QCOMPARE(signalSpy.count(), 0);
      QCOMPARE(dm.rowCount(QModelIndex()), 0);
    } else {
      QVERIFY(dm.initialized());
      QCOMPARE(signalSpy.count(), 1);

      QFETCH(int, devices);
      QCOMPARE(dm.rowCount(QModelIndex()), devices);
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::NameRole), QVariant());
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::CurrentOneRole), QVariant());
      QCOMPARE(dm.data(QModelIndex(), DeviceModel::CreatedAtRole), QVariant());

      QModelIndex index = dm.index(0, 0);

      QFETCH(QVariant, deviceName);
      QCOMPARE(dm.data(index, DeviceModel::NameRole), deviceName);

      // We cannot compare the currentOne with the DM because the Keys object
      // doesn't exist in the MozillaVPN mock object
      // QFETCH(QVariant, currentOne);
      // QCOMPARE(dm.data(index, DeviceModel::CurrentOneRole), currentOne);
      // QCOMPARE(!!dm.currentDevice(&keys), currentOne.toBool());

      QFETCH(QVariant, createdAt);
      QCOMPARE(dm.data(index, DeviceModel::CreatedAtRole), createdAt);

      QCOMPARE(dm.activeDevices(), devices);

      Keys keys;
      keys.storeKeys("private", "currentDevicePubkey");

      if (devices > 0) {
        QVERIFY(dm.hasDevice(deviceName.toString()));
        QVERIFY(dm.device(deviceName.toString()) != nullptr);

        dm.removeDevice("FOO");
        QCOMPARE(dm.activeDevices(), devices);

        dm.removeDevice(deviceName.toString());
        QCOMPARE(dm.activeDevices(), devices - 1);
      }
    }
  }
}

// Keys
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::keysBasic() {
  Keys k;
  QVERIFY(!k.initialized());
  QCOMPARE(k.privateKey(), "");

  k.storeKeys("private", "public");
  QVERIFY(k.initialized());
  QCOMPARE(k.privateKey(), "private");
  QCOMPARE(k.publicKey(), "public");

  k.forgetKeys();
  QVERIFY(!k.initialized());
  QCOMPARE(k.privateKey(), "");
  QCOMPARE(k.publicKey(), "");

  // Private and public keys in the settings.
  {
    SettingsHolder settingsHolder;

    QCOMPARE(k.fromSettings(), false);

    SettingsHolder::instance()->setPrivateKey("WOW");
    QCOMPARE(k.fromSettings(), false);

    SettingsHolder::instance()->setPublicKey("WOW2");
    QCOMPARE(k.fromSettings(), true);
  }

  // No public keys, but we can retrieve it from the devices.
  {
    SettingsHolder settingsHolder;

    QCOMPARE(k.fromSettings(), false);

    QJsonObject d;
    d.insert("name", Device::currentDeviceName());
    d.insert("pubkey", "devicePubkey");
    d.insert("created_at", "2017-07-24T15:46:29");
    d.insert("ipv4_address", "deviceIpv4");
    d.insert("ipv6_address", "deviceIpv6");

    QJsonArray devices;
    devices.append(d);

    QJsonObject obj;
    obj.insert("devices", devices);

    SettingsHolder::instance()->setDevices(QJsonDocument(obj).toJson());

    SettingsHolder::instance()->setPrivateKey("WOW");
    QCOMPARE(k.fromSettings(), true);
  }
}

// Server
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverBasic() {
  Server s;
  QVERIFY(!s.initialized());
  QCOMPARE(s.hostname(), "");
  QCOMPARE(s.ipv4AddrIn(), "");
  QCOMPARE(s.ipv4Gateway(), "");
  QCOMPARE(s.ipv6AddrIn(), "");
  QCOMPARE(s.ipv6Gateway(), "");
  QCOMPARE(s.publicKey(), "");
  QCOMPARE(s.weight(), (uint32_t)0);
  QCOMPARE(s.choosePort(), (uint32_t)0);
}

void TestModels::serverFromJson_data() {
  QTest::addColumn<QJsonObject>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("hostname");
  QTest::addColumn<QString>("ipv4AddrIn");
  QTest::addColumn<QString>("ipv4Gateway");
  QTest::addColumn<QString>("ipv6AddrIn");
  QTest::addColumn<QString>("ipv6Gateway");
  QTest::addColumn<QString>("publicKey");
  QTest::addColumn<int>("weight");
  QTest::addColumn<QList<int>>("ports");

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
                              << "publicKey" << 1234 << QList<int>{0};

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
  portRange.replace(1, "B");
  portRanges.replace(0, portRange);
  obj.insert("port_ranges", portRanges);
  QTest::addRow("all good") << obj << false;

  portRange.replace(0, 42);
  portRange.replace(1, 42);
  portRanges.replace(0, portRange);
  obj.insert("port_ranges", portRanges);
  QTest::addRow("all good") << obj << true << "hostname"
                            << "ipv4AddrIn"
                            << "ipv4Gateway"
                            << "ipv6AddrIn"
                            << "ipv6Gateway"
                            << "publicKey" << 1234 << QList<int>{42};

  portRange.replace(0, 42);
  portRange.replace(1, 43);
  portRanges.replace(0, portRange);
  obj.insert("port_ranges", portRanges);
  QTest::addRow("all good") << obj << true << "hostname"
                            << "ipv4AddrIn"
                            << "ipv4Gateway"
                            << "ipv6AddrIn"
                            << "ipv6Gateway"
                            << "publicKey" << 1234 << QList<int>{42, 43};
}

void TestModels::serverFromJson() {
  QFETCH(QJsonObject, json);
  QFETCH(bool, result);

  Server s;
  QCOMPARE(s.fromJson(json), result);

  if (!result) {
    QVERIFY(!s.initialized());
    return;
  }

  QVERIFY(s.initialized());

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

  QFETCH(QList<int>, ports);
  Q_ASSERT(ports.length() >= 1);
  if (ports.length() == 1) {
    QCOMPARE(s.choosePort(), (uint32_t)ports[0]);
  } else {
    QVERIFY(ports.contains(s.choosePort()));
  }

  Server sB(s);
  QCOMPARE(sB.initialized(), s.initialized());
  QCOMPARE(sB.hostname(), s.hostname());
  QCOMPARE(sB.ipv4AddrIn(), s.ipv4AddrIn());
  QCOMPARE(sB.ipv4Gateway(), s.ipv4Gateway());
  QCOMPARE(sB.ipv6AddrIn(), s.ipv6AddrIn());
  QCOMPARE(sB.ipv6Gateway(), s.ipv6Gateway());
  QCOMPARE(sB.publicKey(), s.publicKey());
  QCOMPARE(sB.weight(), s.weight());

  Server sC;
  sC = s;
  QCOMPARE(sC.initialized(), s.initialized());
  QCOMPARE(sC.hostname(), s.hostname());
  QCOMPARE(sC.ipv4AddrIn(), s.ipv4AddrIn());
  QCOMPARE(sC.ipv4Gateway(), s.ipv4Gateway());
  QCOMPARE(sC.ipv6AddrIn(), s.ipv6AddrIn());
  QCOMPARE(sC.ipv6Gateway(), s.ipv6Gateway());
  QCOMPARE(sC.publicKey(), s.publicKey());
  QCOMPARE(sC.weight(), s.weight());

  s = s;
}

void TestModels::serverWeightChooser() {
  QList<Server> list;
  list.append(Server());

  const Server& s = Server::weightChooser(list);
  QCOMPARE(&s, &list[0]);
}

// ServerCity
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverCityBasic() {
  ServerCity sc;
  QCOMPARE(sc.name(), "");
  QCOMPARE(sc.code(), "");
  QVERIFY(sc.servers().isEmpty());
}

void TestModels::serverCityFromJson_data() {
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
  QTest::addRow("servers invalid 1") << obj << false;

  QJsonArray servers;
  obj.insert("servers", servers);
  QTest::addRow("servers empty") << obj << true << "name"
                                 << "code" << 0;

  servers.append(42);
  obj.insert("servers", servers);
  QTest::addRow("servers invalid 2") << obj << false;

  QJsonObject server;
  servers.replace(0, server);
  obj.insert("servers", servers);
  QTest::addRow("servers invalid 3") << obj << false;

  server.insert("hostname", "hostname");
  server.insert("ipv4_addr_in", "ipv4AddrIn");
  server.insert("ipv4_gateway", "ipv4Gateway");
  server.insert("ipv6_addr_in", "ipv6AddrIn");
  server.insert("ipv6_gateway", "ipv6Gateway");
  server.insert("public_key", "publicKey");
  server.insert("weight", 1234);

  QJsonArray portRanges;
  server.insert("port_ranges", portRanges);

  servers.replace(0, server);
  obj.insert("servers", servers);
  QTest::addRow("servers ok") << obj << true << "name"
                              << "code" << 1;
}

void TestModels::serverCityFromJson() {
  QFETCH(QJsonObject, json);
  QFETCH(bool, result);

  ServerCity sc;
  QCOMPARE(sc.fromJson(json), result);
  if (!result) {
    QCOMPARE(sc.name(), "");
    QCOMPARE(sc.code(), "");
    QVERIFY(sc.servers().isEmpty());
    return;
  }

  QFETCH(QString, name);
  QCOMPARE(sc.name(), name);

  QFETCH(QString, code);
  QCOMPARE(sc.code(), code);

  QFETCH(int, servers);
  QCOMPARE(sc.servers().length(), servers);

  ServerCity scB(sc);
  QCOMPARE(scB.name(), sc.name());
  QCOMPARE(scB.code(), sc.code());

  ServerCity scC;
  scC = sc;
  QCOMPARE(scC.name(), sc.name());
  QCOMPARE(scC.code(), sc.code());

  sc = sc;
}

// ServerCountry
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverCountryBasic() {
  ServerCountry sc;
  QCOMPARE(sc.name(), "");
  QCOMPARE(sc.code(), "");
  QVERIFY(sc.cities().isEmpty());
}

void TestModels::serverCountryFromJson_data() {
  QTest::addColumn<QJsonObject>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("code");
  QTest::addColumn<int>("cities");

  QJsonObject obj;
  QTest::addRow("empty") << obj << false;

  obj.insert("name", "name");
  QTest::addRow("name") << obj << false;

  obj.insert("code", "code");
  QTest::addRow("code") << obj << false;

  obj.insert("cities", "cities");
  QTest::addRow("cities invalid") << obj << false;

  QJsonArray cities;
  obj.insert("cities", cities);
  QTest::addRow("cities empty") << obj << true << "name"
                                << "code" << 0;
}

void TestModels::serverCountryFromJson() {
  QFETCH(QJsonObject, json);
  QFETCH(bool, result);

  ServerCountry sc;
  QCOMPARE(sc.fromJson(json), result);
  if (!result) {
    QCOMPARE(sc.name(), "");
    QCOMPARE(sc.code(), "");
    QVERIFY(sc.cities().isEmpty());
    return;
  }

  QFETCH(QString, name);
  QCOMPARE(sc.name(), name);

  QFETCH(QString, code);
  QCOMPARE(sc.code(), code);

  QFETCH(int, cities);
  QCOMPARE(sc.cities().length(), cities);

  ServerCountry scB(sc);
  QCOMPARE(scB.name(), sc.name());
  QCOMPARE(scB.code(), sc.code());

  ServerCountry scC;
  scC = sc;
  QCOMPARE(scC.name(), sc.name());
  QCOMPARE(scC.code(), sc.code());

  sc = sc;
}

// ServerCountryModel
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverCountryModelBasic() {
  ServerCountryModel dm;
  QVERIFY(!dm.initialized());

  SettingsHolder settingsHolder;

  QVERIFY(!dm.fromSettings());

  QHash<int, QByteArray> rn = dm.roleNames();
  QCOMPARE(rn.count(), 4);
  QCOMPARE(rn[ServerCountryModel::NameRole], "name");
  QCOMPARE(rn[ServerCountryModel::LocalizedNameRole], "localizedName");
  QCOMPARE(rn[ServerCountryModel::CodeRole], "code");
  QCOMPARE(rn[ServerCountryModel::CitiesRole], "cities");

  QCOMPARE(dm.rowCount(QModelIndex()), 0);
  QCOMPARE(dm.data(QModelIndex(), ServerCountryModel::NameRole), QVariant());
}

void TestModels::serverCountryModelFromJson_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<int>("countries");
  QTest::addColumn<QVariant>("name");
  QTest::addColumn<QVariant>("code");
  QTest::addColumn<QVariant>("cities");

  QTest::addRow("invalid") << QByteArray("") << false;
  QTest::addRow("array") << QByteArray("[]") << false;

  QJsonObject obj;
  QTest::addRow("empty") << QJsonDocument(obj).toJson() << false;

  obj.insert("countries", 42);
  QTest::addRow("invalid countries") << QJsonDocument(obj).toJson() << false;

  QJsonArray countries;
  obj.insert("countries", countries);
  QTest::addRow("good but empty") << QJsonDocument(obj).toJson() << true << 0
                                  << QVariant() << QVariant() << QVariant();

  countries.append(42);
  obj.insert("countries", countries);
  QTest::addRow("invalid city") << QJsonDocument(obj).toJson() << false;

  QJsonObject d;
  d.insert("name", "serverCountryName");
  d.insert("code", "serverCountryCode");
  d.insert("cities", QJsonArray());

  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("good but empty cities")
      << QJsonDocument(obj).toJson() << true << 1
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(QStringList{});

  QJsonArray cities;
  cities.append(42);

  d.insert("cities", cities);
  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("invalid city object") << QJsonDocument(obj).toJson() << false;

  QJsonObject city;
  city.insert("code", "serverCityCode");
  city.insert("name", "serverCityName");
  city.insert("servers", QJsonArray());

  cities.replace(0, city);

  d.insert("cities", cities);
  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("good but empty cities")
      << QJsonDocument(obj).toJson() << true << 1
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(
             QList<QVariant>{QStringList{"serverCityName", "serverCityName"}});

  cities.append(city);
  d.insert("cities", cities);
  countries.append(d);
  obj.insert("countries", countries);
  QTest::addRow("good") << QJsonDocument(obj).toJson() << true << 2
                        << QVariant("serverCountryName")
                        << QVariant("serverCountryCode")
                        << QVariant(QList<QVariant>{QStringList{
                               "serverCityName", "serverCityName"}});
}

void TestModels::serverCountryModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // from json
  {
    SettingsHolder settingsHolder;

    ServerCountryModel m;
    QCOMPARE(m.fromJson(json), result);

    if (!result) {
      QVERIFY(!m.initialized());
      QCOMPARE(m.rowCount(QModelIndex()), 0);
    } else {
      QVERIFY(m.initialized());

      QFETCH(int, countries);
      QCOMPARE(m.rowCount(QModelIndex()), countries);

      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::NameRole), QVariant());
      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::CodeRole), QVariant());
      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::CitiesRole),
               QVariant());

      QModelIndex index = m.index(0, 0);

      QFETCH(QVariant, name);
      QCOMPARE(m.data(index, ServerCountryModel::NameRole), name);

      QFETCH(QVariant, code);
      QCOMPARE(m.data(index, ServerCountryModel::CodeRole), code);

      QFETCH(QVariant, cities);
      QCOMPARE(m.data(index, ServerCountryModel::CitiesRole), cities);

      QCOMPARE(m.countryName(code.toString()), name.toString());
      QCOMPARE(m.countryName("invalid"), QString());

      QVERIFY(m.fromJson(json));
    }
  }

  // from settings
  {
    SettingsHolder settingsHolder;

    SettingsHolder::instance()->setServers(json);

    ServerCountryModel m;
    QCOMPARE(m.fromSettings(), result);

    if (!result) {
      QVERIFY(!m.initialized());
      QCOMPARE(m.rowCount(QModelIndex()), 0);
    } else {
      QVERIFY(m.initialized());

      QFETCH(int, countries);
      QCOMPARE(m.rowCount(QModelIndex()), countries);

      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::NameRole), QVariant());
      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::CodeRole), QVariant());
      QCOMPARE(m.data(QModelIndex(), ServerCountryModel::CitiesRole),
               QVariant());

      QModelIndex index = m.index(0, 0);

      QFETCH(QVariant, name);
      QCOMPARE(m.data(index, ServerCountryModel::NameRole), name);

      QFETCH(QVariant, code);
      QCOMPARE(m.data(index, ServerCountryModel::CodeRole), code);

      QFETCH(QVariant, cities);
      QCOMPARE(m.data(index, ServerCountryModel::CitiesRole), cities);

      QCOMPARE(m.data(index, ServerCountryModel::CitiesRole + 1), QVariant());

      QCOMPARE(m.countryName(code.toString()), name.toString());
      QCOMPARE(m.countryName("invalid"), QString());
    }
  }
}

void TestModels::serverCountryModelPick() {
  QJsonObject server;
  server.insert("hostname", "hostname");
  server.insert("ipv4_addr_in", "ipv4AddrIn");
  server.insert("ipv4_gateway", "ipv4Gateway");
  server.insert("ipv6_addr_in", "ipv6AddrIn");
  server.insert("ipv6_gateway", "ipv6Gateway");
  server.insert("public_key", "publicKey");
  server.insert("weight", 1234);
  server.insert("port_ranges", QJsonArray());

  QJsonArray servers;
  servers.append(server);

  QJsonObject city;
  city.insert("code", "serverCityCode");
  city.insert("name", "serverCityName");
  city.insert("servers", servers);

  QJsonArray cities;
  cities.append(city);

  QJsonObject country;
  country.insert("name", "serverCountryName");
  country.insert("code", "serverCountryCode");
  country.insert("cities", cities);

  QJsonArray countries;
  countries.append(country);

  QJsonObject obj;
  obj.insert("countries", countries);

  QByteArray json = QJsonDocument(obj).toJson();

  ServerCountryModel m;
  QCOMPARE(m.fromJson(json), true);

  {
    ServerData sd;
    QCOMPARE(m.pickIfExists("serverCountryCode", "serverCityCode", sd), true);
    QCOMPARE(sd.countryCode(), "serverCountryCode");
    QCOMPARE(sd.countryName(), "serverCountryName");
    QCOMPARE(sd.cityName(), "serverCityName");
    QCOMPARE(m.exists(sd), true);

    QCOMPARE(m.pickIfExists("serverCountryCode2", "serverCityCode", sd), false);
    QCOMPARE(m.pickIfExists("serverCountryCode", "serverCityCode2", sd), false);
  }

  {
    ServerData sd;
    m.pickRandom(sd);
    QCOMPARE(sd.countryCode(), "serverCountryCode");
    QCOMPARE(sd.countryName(), "serverCountryName");
    QCOMPARE(sd.cityName(), "serverCityName");
    QCOMPARE(m.exists(sd), true);
  }

  {
    ServerData sd;
    QCOMPARE(m.pickByIPv4Address("ipv4AddrIn", sd), true);
    QCOMPARE(sd.countryCode(), "serverCountryCode");
    QCOMPARE(sd.countryName(), "serverCountryName");
    QCOMPARE(sd.cityName(), "serverCityName");
    QCOMPARE(m.exists(sd), true);

    QCOMPARE(m.pickByIPv4Address("ipv4AddrIn2", sd), false);
  }
}

// ServerData
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverDataBasic() {
  ServerData sd;
  QSignalSpy spy(&sd, &ServerData::changed);

  QVERIFY(!sd.initialized());
  QCOMPARE(sd.countryCode(), "");
  QCOMPARE(sd.countryName(), "");
  QCOMPARE(sd.cityName(), "");

  {
    QJsonObject countryObj;
    countryObj.insert("name", "serverCountryName");
    countryObj.insert("code", "serverCountryCode");
    countryObj.insert("cities", QJsonArray());
    ServerCountry country;
    QVERIFY(country.fromJson(countryObj));

    QJsonObject cityObj;
    cityObj.insert("code", "serverCityCode");
    cityObj.insert("name", "serverCityName");
    cityObj.insert("servers", QJsonArray());

    ServerCity city;
    QVERIFY(city.fromJson(cityObj));

    sd.initialize(country, city);
    QCOMPARE(spy.count(), 1);

    QVERIFY(sd.initialized());
    QCOMPARE(sd.countryCode(), "serverCountryCode");
    QCOMPARE(sd.countryName(), "serverCountryName");
    QCOMPARE(sd.cityName(), "serverCityName");

    {
      SettingsHolder settingsHolder;

      sd.writeSettings();

      ServerData sd2;
      QVERIFY(sd2.fromSettings());
      QVERIFY(sd2.initialized());
      QCOMPARE(sd2.countryCode(), "serverCountryCode");
      QCOMPARE(sd2.countryName(), "serverCountryName");
      QCOMPARE(sd2.cityName(), "serverCityName");

      QCOMPARE(spy.count(), 1);
    }
  }

  sd.update("new Country Code", "new Country", "new City");
  QCOMPARE(spy.count(), 2);

  QVERIFY(sd.initialized());
  QCOMPARE(sd.countryCode(), "new Country Code");
  QCOMPARE(sd.countryName(), "new Country");
  QCOMPARE(sd.cityName(), "new City");

  sd.forget();
  QCOMPARE(spy.count(), 2);

  QVERIFY(!sd.initialized());
  QCOMPARE(sd.countryCode(), "new Country Code");
  QCOMPARE(sd.countryName(), "new Country");
  QCOMPARE(sd.cityName(), "new City");

  {
    SettingsHolder settingsHolder;
    QVERIFY(!sd.fromSettings());
    QCOMPARE(spy.count(), 2);
  }
}

// User
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::userBasic() {
  User user;
  QVERIFY(!user.initialized());
  QCOMPARE(user.avatar(), "");
  QCOMPARE(user.displayName(), "");
  QCOMPARE(user.email(), "");
  QCOMPARE(user.maxDevices(), 5);
  QVERIFY(!user.subscriptionNeeded());
}

void TestModels::userFromJson_data() {
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
  QTest::newRow("empty subscription")
      << QJsonDocument(obj).toJson() << true << "avatar"
      << "displayName"
      << "email" << 123 << true;

  subscription.insert("vpn", QJsonValue("WOW"));
  obj.insert("subscriptions", subscription);
  QTest::newRow("invalid vpn subscription")
      << QJsonDocument(obj).toJson() << false;

  QJsonObject subVpn;
  subscription.insert("vpn", subVpn);
  obj.insert("subscriptions", subscription);
  QTest::newRow("empty vpn subscription")
      << QJsonDocument(obj).toJson() << false;

  subVpn.insert("active", QJsonValue("sure!"));
  subscription.insert("vpn", subVpn);
  obj.insert("subscriptions", subscription);
  QTest::newRow("invalid active vpn subscription")
      << QJsonDocument(obj).toJson() << false;

  subVpn.insert("active", QJsonValue(true));
  subscription.insert("vpn", subVpn);
  obj.insert("subscriptions", subscription);
  QTest::newRow("active vpn subscription")
      << QJsonDocument(obj).toJson() << true << "avatar"
      << "displayName"
      << "email" << 123 << false;

  subVpn.insert("active", QJsonValue(false));
  subscription.insert("vpn", subVpn);
  obj.insert("subscriptions", subscription);
  QTest::newRow("inactive vpn subscription")
      << QJsonDocument(obj).toJson() << true << "avatar"
      << "displayName"
      << "email" << 123 << true;
}

void TestModels::userFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  User user;
  QSignalSpy spy(&user, &User::changed);
  QCOMPARE(user.fromJson(json), result);

  if (!result) {
    QVERIFY(!user.initialized());
    QCOMPARE(spy.count(), 0);

    QCOMPARE(user.avatar(), "");
    QCOMPARE(user.displayName(), "");
    QCOMPARE(user.email(), "");
    QCOMPARE(user.maxDevices(), 5);
    QVERIFY(!user.subscriptionNeeded());
    return;
  }

  QVERIFY(user.initialized());
  QCOMPARE(spy.count(), 1);

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

  {
    SettingsHolder settingsHolder;
    user.writeSettings();

    // FromSettings
    {
      User user;
      QSignalSpy spy(&user, &User::changed);

      QVERIFY(user.fromSettings());
      QVERIFY(user.initialized());
      QCOMPARE(spy.count(), 0);

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
  }
}

void TestModels::userFromSettings() {
  SettingsHolder settingsHolder;

  User user;
  QSignalSpy spy(&user, &User::changed);

  QVERIFY(!user.fromSettings());
  QVERIFY(!user.initialized());
  QCOMPARE(spy.count(), 0);

  SettingsHolder::instance()->setUserAvatar("avatar");
  QVERIFY(!user.fromSettings());
  QVERIFY(!user.initialized());
  QCOMPARE(spy.count(), 0);

  SettingsHolder::instance()->setUserDisplayName("displayName");
  QVERIFY(!user.fromSettings());
  QVERIFY(!user.initialized());
  QCOMPARE(spy.count(), 0);

  SettingsHolder::instance()->setUserEmail("email");
  QVERIFY(!user.fromSettings());
  QVERIFY(!user.initialized());
  QCOMPARE(spy.count(), 0);

  SettingsHolder::instance()->setUserMaxDevices(123);
  QVERIFY(!user.fromSettings());
  QVERIFY(!user.initialized());
  QCOMPARE(spy.count(), 0);

  SettingsHolder::instance()->setUserSubscriptionNeeded(true);
  QVERIFY(user.fromSettings());
  QVERIFY(user.initialized());
  QCOMPARE(spy.count(), 0);
  QCOMPARE(user.avatar(), "avatar");
  QCOMPARE(user.displayName(), "displayName");
  QCOMPARE(user.email(), "email");
  QCOMPARE(user.maxDevices(), 123);
  QCOMPARE(user.subscriptionNeeded(), true);
}

// IPAddressRange
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::ipAddressRangeBasic() {
  IPAddressRange a("ip", (uint32_t)123, IPAddressRange::IPv4);
  QCOMPARE(a.ipAddress(), "ip");
  QCOMPARE(a.range(), (uint32_t)123);
  QCOMPARE(a.type(), IPAddressRange::IPv4);
  QCOMPARE(a.toString(), "ip/123");

  IPAddressRange b(a);
  QCOMPARE(b.ipAddress(), a.ipAddress());
  QCOMPARE(b.range(), a.range());
  QCOMPARE(b.type(), a.type());
  QCOMPARE(b.toString(), a.toString());

  IPAddressRange c(a);
  c = a;
  QCOMPARE(c.ipAddress(), a.ipAddress());
  QCOMPARE(c.range(), a.range());
  QCOMPARE(c.type(), a.type());
  QCOMPARE(c.toString(), a.toString());

  a = a;
}

static TestModels s_testModels;
