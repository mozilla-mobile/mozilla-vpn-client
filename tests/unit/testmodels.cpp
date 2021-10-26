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
#include "../../src/models/surveymodel.h"
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
  QCOMPARE(device.uniqueId(), "");
  QCOMPARE(device.createdAt(), QDateTime());
  QCOMPARE(device.publicKey(), "");
  QCOMPARE(device.ipv4Address(), "");
  QCOMPARE(device.ipv6Address(), "");
  QVERIFY(!device.isDeviceForMigration("name"));
}

void TestModels::deviceCurrentDeviceName() {
  QVERIFY(!Device::currentDeviceName().isEmpty());
}

void TestModels::deviceFromJson_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("uniqueId");
  QTest::addColumn<QString>("publicKey");
  QTest::addColumn<QDateTime>("createdAt");
  QTest::addColumn<QString>("ipv4Address");
  QTest::addColumn<QString>("ipv6Address");

  QJsonObject obj;
  obj.insert("test", "");
  QTest::addRow("null") << QJsonDocument(obj).toJson() << false << ""
                        << ""
                        << "" << QDateTime() << ""
                        << "";

  QJsonObject d;
  obj.insert("test", d);
  QTest::addRow("empty") << QJsonDocument(obj).toJson() << false << ""
                         << ""
                         << "" << QDateTime() << ""
                         << "";

  d.insert("name", "deviceName");
  obj.insert("test", d);
  QTest::addRow("name") << QJsonDocument(obj).toJson() << false << ""
                        << ""
                        << "" << QDateTime() << ""
                        << "";

  d.insert("unique_id", "uniqueId");
  obj.insert("test", d);
  QTest::addRow("name") << QJsonDocument(obj).toJson() << false << ""
                        << ""
                        << "" << QDateTime() << ""
                        << "";

  d.insert("pubkey", "devicePubkey");
  obj.insert("test", d);
  QTest::addRow("pubKey") << QJsonDocument(obj).toJson() << false << ""
                          << ""
                          << "" << QDateTime() << ""
                          << "";

  d.insert("created_at", 42);
  obj.insert("test", d);
  QTest::addRow("createdAt (invalid)")
      << QJsonDocument(obj).toJson() << false << ""
      << ""
      << "" << QDateTime() << ""
      << "";

  d.insert("created_at", "42");
  obj.insert("test", d);
  QTest::addRow("createdAt (invalid string)")
      << QJsonDocument(obj).toJson() << false << ""
      << ""
      << "" << QDateTime() << ""
      << "";

  d.insert("created_at", "2017-07-24T15:46:29");
  obj.insert("test", d);
  QTest::addRow("createdAt") << QJsonDocument(obj).toJson() << false << ""
                             << ""
                             << "" << QDateTime() << ""
                             << "";

  d.insert("ipv4_address", "deviceIpv4");
  obj.insert("test", d);
  QTest::addRow("ipv4Address") << QJsonDocument(obj).toJson() << false << ""
                               << ""
                               << "" << QDateTime() << ""
                               << "";

  d.insert("ipv6_address", "deviceIpv6");
  obj.insert("test", d);
  QTest::addRow("ipv6Address")
      << QJsonDocument(obj).toJson() << true << "deviceName"
      << "uniqueId"
      << "devicePubkey"
      << QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate)
      << "deviceIpv4"
      << "deviceIpv6";

  d.remove("unique_id");
  obj.insert("test", d);
  QTest::addRow("no unique_id")
      << QJsonDocument(obj).toJson() << true << "deviceName"
      << ""
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

  QFETCH(QString, uniqueId);
  QCOMPARE(device.uniqueId(), uniqueId);

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
  QCOMPARE(deviceB.uniqueId(), device.uniqueId());
  QCOMPARE(deviceB.createdAt(), device.createdAt());
  QCOMPARE(deviceB.publicKey(), device.publicKey());
  QCOMPARE(deviceB.ipv4Address(), device.ipv4Address());
  QCOMPARE(deviceB.ipv6Address(), device.ipv6Address());

  Device deviceC;
  deviceC = device;
  QCOMPARE(deviceC.name(), device.name());
  QCOMPARE(deviceC.uniqueId(), device.uniqueId());
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
  dm.removeDeviceFromPublicKey("foo");
  QCOMPARE(dm.deviceFromPublicKey("foo"), nullptr);
  QCOMPARE(dm.deviceFromUniqueId(), nullptr);
  QCOMPARE(dm.activeDevices(), 0);

  Keys keys;
  QCOMPARE(dm.currentDevice(&keys), nullptr);

  QHash<int, QByteArray> rn = dm.roleNames();
  QCOMPARE(rn.count(), 4);
  QCOMPARE(rn[DeviceModel::NameRole], "name");
  QCOMPARE(rn[DeviceModel::PublicKeyRole], "publicKey");
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
  QTest::addColumn<QVariant>("devicePublicKey");
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
      << QJsonDocument(obj).toJson() << true << 0 << QVariant() << QVariant()
      << QVariant() << QVariant();

  devices.append(42);
  obj.insert("devices", devices);
  QTest::addRow("invalid devices") << QJsonDocument(obj).toJson() << false;

  QJsonObject d;
  d.insert("name", "deviceName");
  d.insert("unique_id", Device::uniqueDeviceId());
  d.insert("pubkey", "devicePubkey");
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.replace(0, d);
  obj.insert("devices", devices);
  QTest::addRow("good") << QJsonDocument(obj).toJson() << true << 1
                        << QVariant("deviceName") << QVariant("devicePubkey")
                        << QVariant(false)
                        << QVariant(QDateTime::fromString("2017-07-24T15:46:29",
                                                          Qt::ISODate));

  d.insert("name", Device::currentDeviceName());
  d.insert("unique_id", "43");
  d.insert("pubkey", "currentDevicePubkey");
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.append(d);
  obj.insert("devices", devices);
  QTest::addRow("good - 2 devices")
      << QJsonDocument(obj).toJson() << true << 2
      << QVariant(Device::currentDeviceName())
      << QVariant("currentDevicePubkey") << QVariant(true)
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
      QFETCH(QVariant, devicePublicKey);
      QCOMPARE(dm.data(index, DeviceModel::NameRole), deviceName);
      QCOMPARE(dm.data(index, DeviceModel::PublicKeyRole), devicePublicKey);

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
        QVERIFY(dm.deviceFromPublicKey(devicePublicKey.toString()) != nullptr);
        QVERIFY(dm.deviceFromUniqueId() != nullptr);

        dm.removeDeviceFromPublicKey("FOO");
        QCOMPARE(dm.activeDevices(), devices);

        dm.removeDeviceFromPublicKey(devicePublicKey.toString());
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
      QFETCH(QVariant, devicePublicKey);
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
        QVERIFY(dm.deviceFromPublicKey(devicePublicKey.toString()) != nullptr);
        QVERIFY(dm.deviceFromUniqueId() != nullptr);

        dm.removeDeviceFromPublicKey("FOO");
        QCOMPARE(dm.activeDevices(), devices);

        dm.removeDeviceFromPublicKey(devicePublicKey.toString());
        QCOMPARE(dm.activeDevices(), devices - 1);
      }
    }
  }
}

void TestModels::deviceModelRemoval() {
  QJsonObject d1;
  d1.insert("name", "deviceName");
  d1.insert("unique_id", "d1");
  d1.insert("pubkey", "devicePubkey1");
  d1.insert("created_at", "2017-07-24T15:46:29");
  d1.insert("ipv4_address", "deviceIpv4");
  d1.insert("ipv6_address", "deviceIpv6");

  QJsonObject d2;
  d2.insert("name", "deviceName");
  d2.insert("unique_id", "d2");
  d2.insert("pubkey", "devicePubkey2");
  d2.insert("created_at", "2017-07-24T15:46:29");
  d2.insert("ipv4_address", "deviceIpv4");
  d2.insert("ipv6_address", "deviceIpv6");

  QJsonArray devices;
  devices.append(d1);
  devices.append(d2);

  QJsonObject obj;
  obj.insert("devices", devices);

  Keys keys;
  keys.storeKeys("private", "currentDevicePubkey");

  DeviceModel dm;
  QCOMPARE(dm.fromJson(&keys, QJsonDocument(obj).toJson()), true);

  QCOMPARE(dm.rowCount(QModelIndex()), 2);

  // Let's start the removal.
  dm.startDeviceRemovalFromPublicKey("devicePubkey1");
  QCOMPARE(dm.rowCount(QModelIndex()), 1);

  // Refresh the model. The removed device is still gone.
  QCOMPARE(dm.fromJson(&keys, QJsonDocument(obj).toJson()), true);
  QCOMPARE(dm.rowCount(QModelIndex()), 1);

  // Complete the removal without removing the device for real (simulate a
  // failure).
  dm.stopDeviceRemovalFromPublicKey("devicePubkey1", &keys);
  QCOMPARE(dm.rowCount(QModelIndex()), 2);

  // Let's start the removal again.
  dm.startDeviceRemovalFromPublicKey("devicePubkey1");
  QCOMPARE(dm.rowCount(QModelIndex()), 1);

  // Remove the device for real.
  dm.removeDeviceFromPublicKey("devicePubkey1");
  QCOMPARE(dm.rowCount(QModelIndex()), 1);

  // We have only 1 device left.
  dm.stopDeviceRemovalFromPublicKey("devicePubkey1", &keys);
  QCOMPARE(dm.rowCount(QModelIndex()), 1);
}

// Feedback Category
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::feedbackCategoryBasic() {
  FeedbackCategoryModel fcm;

  QHash<int, QByteArray> rn = fcm.roleNames();
  QCOMPARE(rn.count(), 2);
  QCOMPARE(rn[FeedbackCategoryModel::CategoryNameRole], "value");
  QCOMPARE(rn[FeedbackCategoryModel::LocalizedNameRole], "name");

  QVERIFY(fcm.rowCount(QModelIndex()) > 0);
  for (int i = 0; i < fcm.rowCount(QModelIndex()); ++i) {
    QModelIndex index = fcm.index(i, 0);
    QVERIFY(!fcm.data(index, FeedbackCategoryModel::CategoryNameRole)
                 .toString()
                 .isEmpty());
    QVERIFY(!fcm.data(index, FeedbackCategoryModel::LocalizedNameRole)
                 .toString()
                 .isEmpty());
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

  QCOMPARE(s.socksName(), QString());
  QCOMPARE(s.multihopPort(), (uint32_t)0);

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
  QCOMPARE(sB.socksName(), s.socksName());
  QCOMPARE(sB.multihopPort(), s.multihopPort());

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
  QCOMPARE(sC.socksName(), s.socksName());
  QCOMPARE(sC.multihopPort(), s.multihopPort());

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
    QCOMPARE(sd.exitCountryCode(), "serverCountryCode");
    QCOMPARE(sd.exitCityName(), "serverCityName");
    QCOMPARE(m.exists(sd), true);

    QCOMPARE(m.pickIfExists("serverCountryCode2", "serverCityCode", sd), false);
    QCOMPARE(m.pickIfExists("serverCountryCode", "serverCityCode2", sd), false);
  }

  {
    ServerData sd;
    m.pickRandom(sd);
    QCOMPARE(sd.exitCountryCode(), "serverCountryCode");
    QCOMPARE(sd.exitCityName(), "serverCityName");
    QCOMPARE(m.exists(sd), true);
  }

  {
    SettingsHolder settingsHolder;
    QStringList tuple = m.pickRandom();
    QCOMPARE(tuple.length(), 3);
    QCOMPARE(tuple.at(0), "serverCountryCode");
    QCOMPARE(tuple.at(1), "serverCityName");
    QCOMPARE(tuple.at(2), "serverCityName");  // Localized?
  }

  {
    ServerData sd;
    QCOMPARE(m.pickByIPv4Address("ipv4AddrIn", sd), true);
    QCOMPARE(sd.exitCountryCode(), "serverCountryCode");
    QCOMPARE(sd.exitCityName(), "serverCityName");
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
  QCOMPARE(sd.exitCountryCode(), "");
  QCOMPARE(sd.exitCityName(), "");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");

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

    sd.update(country.code(), city.name());
    QCOMPARE(spy.count(), 1);

    QVERIFY(sd.initialized());
    QCOMPARE(sd.exitCountryCode(), "serverCountryCode");
    QCOMPARE(sd.exitCityName(), "serverCityName");
    QVERIFY(!sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "");
    QCOMPARE(sd.entryCityName(), "");
    QCOMPARE(sd.toString(), "serverCityName, serverCountryCode");

    {
      SettingsHolder settingsHolder;

      sd.writeSettings();

      ServerData sd2;
      QVERIFY(sd2.fromSettings());
      QVERIFY(sd2.initialized());
      QCOMPARE(sd2.exitCountryCode(), "serverCountryCode");
      QCOMPARE(sd2.exitCityName(), "serverCityName");
      QVERIFY(!sd2.multihop());
      QCOMPARE(sd2.entryCountryCode(), "");
      QCOMPARE(sd2.entryCityName(), "");
      QCOMPARE(sd2.toString(), "serverCityName, serverCountryCode");

      QCOMPARE(spy.count(), 1);
    }
  }

  sd.update("new Country Code", "new City");
  QCOMPARE(spy.count(), 2);

  QVERIFY(sd.initialized());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");
  QCOMPARE(sd.toString(), "new City, new Country Code");

  sd.forget();
  QCOMPARE(spy.count(), 2);

  QVERIFY(!sd.initialized());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");
  QCOMPARE(sd.toString(), "");

  {
    SettingsHolder settingsHolder;
    QVERIFY(!sd.fromSettings());
    QCOMPARE(spy.count(), 2);
  }

  sd.update("new Country Code", "new City", "entry Country Code", "entry City");
  QVERIFY(sd.initialized());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "entry Country Code");
  QCOMPARE(sd.entryCityName(), "entry City");
  QCOMPARE(sd.toString(),
           "entry City, entry Country Code -> new City, new Country Code");

  sd.forget();
  QCOMPARE(spy.count(), 3);

  QVERIFY(!sd.initialized());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "entry Country Code");
  QCOMPARE(sd.entryCityName(), "entry City");

  sd.forget();
  QVERIFY(sd.fromString("Eureka, CA, us -> McMurdo Station, aq"));
  QVERIFY(sd.initialized());
  QCOMPARE(sd.exitCountryCode(), "aq");
  QCOMPARE(sd.exitCityName(), "McMurdo Station");
  QVERIFY(sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "us");
  QCOMPARE(sd.entryCityName(), "Eureka, CA");
  QCOMPARE(sd.toString(), "Eureka, CA, us -> McMurdo Station, aq");
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

// SurveyModel
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::surveyModelFromJson_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<int>("surveys");
  QTest::addColumn<QString>("surveyId");
  QTest::addColumn<QString>("surveyUrl");
  QTest::addColumn<int>("surveyTriggerTime");
  QTest::addColumn<bool>("surveyTriggerable");

  QTest::addRow("invalid") << QByteArray("") << false;
  QTest::addRow("object") << QByteArray("{}") << false;

  QJsonArray surveys;
  QTest::addRow("good but empty")
      << QJsonDocument(surveys).toJson() << true << 0 << ""
      << "" << 0;

  surveys.append(42);
  QTest::addRow("invalid surveys") << QJsonDocument(surveys).toJson() << false;

  QJsonObject d;
  surveys.replace(0, d);
  QTest::addRow("survey without id")
      << QJsonDocument(surveys).toJson() << false;

  d.insert("id", "A");
  surveys.replace(0, d);
  QTest::addRow("survey without id")
      << QJsonDocument(surveys).toJson() << false;

  d.insert("url", "http://vpn.mozilla.org");
  surveys.replace(0, d);
  QTest::addRow("survey without id")
      << QJsonDocument(surveys).toJson() << false;

  d.insert("trigger_time", 0);
  surveys.replace(0, d);
  QTest::addRow("good") << QJsonDocument(surveys).toJson() << true << 1 << "A"
                        << "http://vpn.mozilla.org" << 0 << true;

  d.insert("platforms", 12345);
  surveys.replace(0, d);
  QTest::addRow("bogus platforms") << QJsonDocument(surveys).toJson() << false;

  QJsonArray platforms = {"ios", "android"};
  d.insert("platforms", platforms);
  surveys.replace(0, d);
  QTest::addRow("unmatched platforms")
      << QJsonDocument(surveys).toJson() << true << 1 << "A"
      << "http://vpn.mozilla.org" << 0 << false;

  platforms.append("dummy");
  d.insert("platforms", platforms);
  surveys.replace(0, d);
  QTest::addRow("matched platforms")
      << QJsonDocument(surveys).toJson() << true << 1 << "A"
      << "http://vpn.mozilla.org" << 0 << true;

  QJsonObject d2;
  d2.insert("id", "B");
  d2.insert("url", "http://vpn.mozilla.org");
  d2.insert("trigger_time", 1234);

  surveys.append(d2);
  QTest::addRow("good - 2 surveys")
      << QJsonDocument(surveys).toJson() << true << 2 << "A"
      << "http://vpn.mozilla.org" << 0 << true;
}

void TestModels::surveyModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // fromJson
  {
    SurveyModel sm;

    QCOMPARE(sm.fromJson(json), result);

    if (!result) {
      QCOMPARE(sm.surveys().length(), 0);
    } else {
      QFETCH(int, surveys);
      QCOMPARE(sm.surveys().length(), surveys);

      if (surveys > 0) {
        QFETCH(QString, surveyId);
        QCOMPARE(sm.surveys()[0].id(), surveyId);

        QFETCH(QString, surveyUrl);
        QCOMPARE(sm.surveys()[0].url(), surveyUrl);

        QFETCH(int, surveyTriggerTime);
        QCOMPARE((int)sm.surveys()[0].triggerTime(), surveyTriggerTime);

        Survey a(sm.surveys()[0]);
        QCOMPARE(a.id(), surveyId);
        QCOMPARE(a.url(), surveyUrl);
        QCOMPARE((int)a.triggerTime(), surveyTriggerTime);

        Survey b;
        b = a;
        QCOMPARE(b.id(), surveyId);
        QCOMPARE(b.url(), surveyUrl);
        QCOMPARE((int)b.triggerTime(), surveyTriggerTime);

        b = b;
      }

      QVERIFY(sm.fromJson(json));
    }
  }

  // fromSettings
  {
    SettingsHolder settingsHolder;
    SettingsHolder::instance()->setSurveys(json);

    SurveyModel sm;
    QCOMPARE(sm.fromSettings(), result);

    if (!result) {
      QCOMPARE(sm.surveys().length(), 0);
    } else {
      QFETCH(int, surveys);
      QCOMPARE(sm.surveys().length(), surveys);

      if (surveys > 0) {
        QFETCH(QString, surveyId);
        QCOMPARE(sm.surveys()[0].id(), surveyId);

        QFETCH(QString, surveyUrl);
        QCOMPARE(sm.surveys()[0].url(), surveyUrl);

        QFETCH(int, surveyTriggerTime);
        QFETCH(bool, surveyTriggerable);
        QCOMPARE((int)sm.surveys()[0].triggerTime(), surveyTriggerTime);
        QCOMPARE(sm.surveys()[0].isTriggerable(), surveyTriggerable);

        if (surveyTriggerable) {
          QStringList list = settingsHolder.consumedSurveys();
          list.append(surveyId);
          settingsHolder.setConsumedSurveys(list);

          QVERIFY(!sm.surveys()[0].isTriggerable());
        }
      }
    }
  }
}

static TestModels s_testModels;
