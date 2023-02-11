/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmodels.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "appconstants.h"
#include "helper.h"
#include "localizer.h"
#include "models/device.h"
#include "models/devicemodel.h"
#include "models/feedbackcategorymodel.h"
#include "models/keys.h"
#include "models/location.h"
#include "models/recentconnections.h"
#include "models/servercity.h"
#include "models/servercountry.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "models/user.h"
#include "settingsholder.h"

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

// Recent Connections
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::recentConnectionBasic() {
  SettingsHolder settingsHolder;

  RecentConnections* rc = RecentConnections::instance();

  const RecentConnectionModel* rcSingleHop = rc->singleHopModel();
  {
    QHash<int, QByteArray> roles = rcSingleHop->roleNames();
    QCOMPARE(roles[RecentConnectionModel::ExitCountryCodeRole],
             "exitCountryCode");
    QCOMPARE(roles[RecentConnectionModel::ExitCityNameRole], "exitCityName");
    QCOMPARE(roles[RecentConnectionModel::LocalizedExitCityNameRole],
             "localizedExitCityName");
    QCOMPARE(roles[RecentConnectionModel::IsMultiHopRole], "isMultiHop");
    QCOMPARE(roles[RecentConnectionModel::EntryCountryCodeRole],
             "entryCountryCode");
    QCOMPARE(roles[RecentConnectionModel::EntryCityNameRole], "entryCityName");
    QCOMPARE(roles[RecentConnectionModel::LocalizedEntryCityNameRole],
             "localizedEntryCityName");
  }

  const RecentConnectionModel* rcMultiHop = rc->multiHopModel();
  {
    QHash<int, QByteArray> roles = rcMultiHop->roleNames();
    QCOMPARE(roles[RecentConnectionModel::ExitCountryCodeRole],
             "exitCountryCode");
    QCOMPARE(roles[RecentConnectionModel::ExitCityNameRole], "exitCityName");
    QCOMPARE(roles[RecentConnectionModel::LocalizedExitCityNameRole],
             "localizedExitCityName");
    QCOMPARE(roles[RecentConnectionModel::IsMultiHopRole], "isMultiHop");
    QCOMPARE(roles[RecentConnectionModel::EntryCountryCodeRole],
             "entryCountryCode");
    QCOMPARE(roles[RecentConnectionModel::EntryCityNameRole], "entryCityName");
    QCOMPARE(roles[RecentConnectionModel::LocalizedEntryCityNameRole],
             "localizedEntryCityName");
  }

  rc->initialize();

  QVERIFY(rcSingleHop->isEmpty());
  QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 0);

  QVERIFY(rcMultiHop->isEmpty());
  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);

  MozillaVPN::instance()->serverData()->initialize();

  // First entry (single hop)
  {
    MozillaVPN::instance()->serverData()->changeServer("a", "b");

    QVERIFY(rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 0);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);
  }

  // Second entry (single hop)
  {
    MozillaVPN::instance()->serverData()->changeServer("c", "d");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 1);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);

    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  // Reinsering the same connection: no changes
  {
    MozillaVPN::instance()->serverData()->changeServer("c", "d");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 1);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);

    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  // Third entry as the first one: move on top
  {
    MozillaVPN::instance()->serverData()->changeServer("a", "b");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 1);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);

    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  // 4th entry
  {
    MozillaVPN::instance()->serverData()->changeServer("e", "f");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 2);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);

    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");

    index = rcSingleHop->index(1, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  // First multi-hop connection: still empty model
  {
    MozillaVPN::instance()->serverData()->changeServer("c", "d", "e", "f");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 2);

    QVERIFY(rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);
  }

  // Second mult-hop connection
  {
    MozillaVPN::instance()->serverData()->changeServer("a", "b", "c", "d");

    QVERIFY(!rcSingleHop->isEmpty());
    QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 2);

    QVERIFY(!rcMultiHop->isEmpty());
    QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 1);

    QModelIndex index = rcMultiHop->index(0, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "e");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "f");
  }

  // Let's add a few single-hop entries. We cannot reach the max value.
  for (int i = 1; i < 2 * AppConstants::RECENT_CONNECTIONS_MAX_COUNT; ++i) {
    MozillaVPN::instance()->serverData()->changeServer(
        QString("%1").arg('a' + i), QString("%1").arg('b' + i));
  }

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 1);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's read from the settings again.
  rc->initialize();

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 1);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's add a few multi-hop entries. We cannot reach the max value.
  for (int i = 1; i < 2 * AppConstants::RECENT_CONNECTIONS_MAX_COUNT; ++i) {
    MozillaVPN::instance()->serverData()->changeServer(
        QString("%1").arg('a' + i), QString("%1").arg('b' + i),
        QString("%1").arg('c' + 1), QString("%1").arg('d' + 1));
  }

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's read from the settings again.
  rc->initialize();
  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()),
           AppConstants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcMultiHop->isEmpty());
}

void TestModels::recentConnectionMigration_data() {
  QTest::addColumn<QStringList>("data");
  QTest::addColumn<int>("countSingleHop");
  QTest::addColumn<int>("countMultiHop");
  QTest::addColumn<QStringList>("firstValueSingleHop");
  QTest::addColumn<QStringList>("firstValueMultiHop");

  QTest::addRow("empty") << QStringList() << 0 << 0 << QStringList()
                         << QStringList();

  // Invalid strings
  QTest::addRow("invalid part a")
      << QStringList({"a"}) << 0 << 0 << QStringList() << QStringList();
  QTest::addRow("invalid part b")
      << QStringList({"a, b -> c"}) << 0 << 0 << QStringList() << QStringList();

  // Single hop
  QTest::addRow("single-hop")
      << QStringList({"a, b", "c, d"}) << 1 << 0
      << QStringList({"d", "c", "", ""}) << QStringList();
  QTest::addRow("single-hop dup") << QStringList({"a, b", "a, b", "a, b"}) << 0
                                  << 0 << QStringList() << QStringList();
  QTest::addRow("single-hop dup 2")
      << QStringList({"a, b", "c, d", "a, b", "a, b"}) << 1 << 0
      << QStringList({"d", "c", "", ""}) << QStringList();

  // Multi-hop
  QTest::addRow("entry and exit")
      << QStringList({"a, b -> c, d", "e, f -> g, h"}) << 0 << 1
      << QStringList() << QStringList({"h", "g", "f", "e"});

  // Mix
  QTest::addRow("many") << QStringList({"a, b -> c, d", "e, f", "g, h -> i, l",
                                        "m, n", "t, z"})
                        << 2 << 1 << QStringList({"n", "m", "", ""})
                        << QStringList({"l", "i", "h", "g"});
}

void TestModels::recentConnectionMigration() {
  SettingsHolder settingsHolder;

  QFETCH(QStringList, data);
  settingsHolder.setRecentConnectionsDeprecated(data);
  QVERIFY(settingsHolder.hasRecentConnectionsDeprecated());

  RecentConnections* rc = RecentConnections::instance();
  rc->initialize();

  const RecentConnectionModel* rcSingleHop = rc->singleHopModel();
  const RecentConnectionModel* rcMultiHop = rc->multiHopModel();

  QFETCH(int, countSingleHop);
  QCOMPARE(rcSingleHop->rowCount(QModelIndex()), countSingleHop);
  QCOMPARE(rcSingleHop->isEmpty(), !countSingleHop);

  QFETCH(int, countMultiHop);
  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), countMultiHop);
  QCOMPARE(rcMultiHop->isEmpty(), !countMultiHop);

  if (countSingleHop > 0) {
    QFETCH(QStringList, firstValueSingleHop);

    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        firstValueSingleHop[0]);
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             firstValueSingleHop[1]);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        firstValueSingleHop[2]);
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             firstValueSingleHop[3]);
  }

  if (countMultiHop > 0) {
    QFETCH(QStringList, firstValueMultiHop);

    QModelIndex index = rcMultiHop->index(0, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        firstValueMultiHop[0]);
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             firstValueMultiHop[1]);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        firstValueMultiHop[2]);
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             firstValueMultiHop[3]);
  }

  QVERIFY(!settingsHolder.hasRecentConnectionsDeprecated());
}

void TestModels::recentConnectionSaveAndRestore() {
  SettingsHolder settingsHolder;

  RecentConnections* rc = RecentConnections::instance();

  const RecentConnectionModel* rcSingleHop = rc->singleHopModel();
  const RecentConnectionModel* rcMultiHop = rc->multiHopModel();

  rc->initialize();

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 0);
  QVERIFY(rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 0);
  QVERIFY(rcMultiHop->isEmpty());

  MozillaVPN::instance()->serverData()->initialize();

  // Let's populate the models
  {
    MozillaVPN::instance()->serverData()->changeServer("a", "b");
    MozillaVPN::instance()->serverData()->changeServer("c", "d");
    MozillaVPN::instance()->serverData()->changeServer("e", "f");

    MozillaVPN::instance()->serverData()->changeServer("c", "d", "e", "f");
    MozillaVPN::instance()->serverData()->changeServer("a", "b", "c", "d");
    MozillaVPN::instance()->serverData()->changeServer("g", "b", "c", "d");
  }

  // Let's check the data
  QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 2);
  QVERIFY(!rcSingleHop->isEmpty());
  {
    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");

    index = rcSingleHop->index(1, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 2);
  QVERIFY(!rcMultiHop->isEmpty());
  {
    QModelIndex index = rcMultiHop->index(0, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "c");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "d");

    index = rcMultiHop->index(1, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "e");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "f");
  }

  // Let's read from the settings again.
  rc->initialize();

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()), 2);
  QVERIFY(!rcMultiHop->isEmpty());
  {
    QModelIndex index = rcSingleHop->index(0, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");

    index = rcSingleHop->index(1, 0);
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcSingleHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "");
    QCOMPARE(rcSingleHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "");
  }

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 2);
  QVERIFY(!rcMultiHop->isEmpty());
  {
    QModelIndex index = rcMultiHop->index(0, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "a");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "b");
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "c");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "d");

    index = rcMultiHop->index(1, 0);
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::ExitCountryCodeRole),
        "c");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::ExitCityNameRole),
             "d");
    QCOMPARE(
        rcMultiHop->data(index, RecentConnectionModel::EntryCountryCodeRole),
        "e");
    QCOMPARE(rcMultiHop->data(index, RecentConnectionModel::EntryCityNameRole),
             "f");
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

  obj.insert("socks5_name", "socks5_name");
  QTest::addRow("socks5_name") << obj << false;

  obj.insert("multihop_port", 1337);
  QTest::addRow("multihop_port") << obj << false;

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

  QCOMPARE(s.socksName(), "socks5_name");
  QCOMPARE(s.multihopPort(), 1337);

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

  obj.insert("latitude", 12.34);
  QTest::addRow("servers invalid 2") << obj << false;

  obj.insert("longitude", 23.45);
  QTest::addRow("servers invalid 3") << obj << false;

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
  server.insert("multihop_port", 1234);
  server.insert("socks5_name", "socks5_name");

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
  QCOMPARE(sc.fromJson(json, "test"), result);
  if (!result) {
    QCOMPARE(sc.name(), "");
    QCOMPARE(sc.code(), "");
    QCOMPARE(sc.country(), "");
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
  QTest::addRow("good but empty")
      << QJsonDocument(obj).toJson() << true << 0 << QVariant() << QVariant()
      << QVariant(QList<QVariant>());

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
      << QJsonDocument(obj).toJson() << true << 0
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(QList<QVariant>());

  QJsonArray cities;
  cities.append(42);

  d.insert("cities", cities);
  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("invalid city object") << QJsonDocument(obj).toJson() << false;

  QJsonObject city;
  city.insert("code", "serverCityCode");
  city.insert("name", "serverCityName");
  city.insert("latitude", 12.34);
  city.insert("longitude", 34.56);
  city.insert("servers", QJsonArray());

  cities.replace(0, city);

  d.insert("cities", cities);
  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("good with one empty city")
      << QJsonDocument(obj).toJson() << true << 1
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(QList<QVariant>{
             QStringList{"serverCityName", "serverCityName", "0"}});

  QJsonObject server;
  server.insert("hostname", "hostname");
  server.insert("ipv4_addr_in", "ipv4AddrIn");
  server.insert("ipv4_gateway", "ipv4Gateway");
  server.insert("ipv6_addr_in", "ipv6AddrIn");
  server.insert("ipv6_gateway", "ipv6Gateway");
  server.insert("public_key", "publicKey");
  server.insert("weight", 1234);
  server.insert("port_ranges", QJsonArray());
  server.insert("multihop_port", 1234);
  server.insert("socks5_name", "socks5_name");

  city.insert("servers", QJsonArray{server});

  cities.replace(0, city);

  d.insert("cities", cities);
  countries.replace(0, d);
  obj.insert("countries", countries);
  QTest::addRow("good with one city")
      << QJsonDocument(obj).toJson() << true << 1
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(
             QList<QVariant>{QStringList{"serverCityName", "serverCityName"}});

  cities.append(city);
  d.insert("cities", cities);
  countries.append(d);
  obj.insert("countries", countries);
  QTest::addRow("good with two cities")
      << QJsonDocument(obj).toJson() << true << 2
      << QVariant("serverCountryName") << QVariant("serverCountryCode")
      << QVariant(
             QList<QVariant>{QStringList{"serverCityName", "serverCityName"}});
}

void TestModels::serverCountryModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // from json
  {
    SettingsHolder settingsHolder;
    Localizer l;

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

      if (countries > 0) {
        QModelIndex index = m.index(0, 0);

        QFETCH(QVariant, name);
        QCOMPARE(m.data(index, ServerCountryModel::NameRole), name);

        QFETCH(QVariant, code);
        QCOMPARE(m.data(index, ServerCountryModel::CodeRole), code);

        QFETCH(QVariant, cities);
        Q_ASSERT(cities.typeId() == QVariant::List);
        QVariant cityData = m.data(index, ServerCountryModel::CitiesRole);
        QCOMPARE(cityData.typeId(), QVariant::List);
        QCOMPARE(cities.toList().length(), cityData.toList().length());
        for (int i = 0; i < cities.toList().length(); i++) {
          QVERIFY(cityData.toList().at(0).canConvert<ServerCity*>());
          ServerCity* cityObj = cityData.toList().at(i).value<ServerCity*>();
          QStringList cityList = cities.toList().at(i).toStringList();
          QCOMPARE(cityObj->name(), cityList.at(0));
          QCOMPARE(cityObj->localizedName(), cityList.at(1));
        }

        QCOMPARE(m.countryName(code.toString()), name.toString());
        QCOMPARE(m.countryName("invalid"), QString());
      }

      QVERIFY(m.fromJson(json));
    }
  }

  // from settings
  {
    SettingsHolder settingsHolder;
    Localizer l;

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

      if (countries > 0) {
        QModelIndex index = m.index(0, 0);

        QFETCH(QVariant, name);
        QCOMPARE(m.data(index, ServerCountryModel::NameRole), name);

        QFETCH(QVariant, code);
        QCOMPARE(m.data(index, ServerCountryModel::CodeRole), code);

        QFETCH(QVariant, cities);
        QVariant cityData = m.data(index, ServerCountryModel::CitiesRole);
        QCOMPARE(cityData.typeId(), QVariant::List);
        QCOMPARE(cities.toList().length(), cityData.toList().length());
        for (int i = 0; i < cities.toList().length(); i++) {
          QVERIFY(cityData.toList().at(0).canConvert<ServerCity*>());
          ServerCity* cityObj = cityData.toList().at(i).value<ServerCity*>();
          QStringList cityList = cities.toList().at(i).toStringList();
          QCOMPARE(cityObj->name(), cityList.at(0));
          QCOMPARE(cityObj->localizedName(), cityList.at(1));
        }

        QCOMPARE(m.data(index, ServerCountryModel::CitiesRole + 1), QVariant());

        QCOMPARE(m.countryName(code.toString()), name.toString());
        QCOMPARE(m.countryName("invalid"), QString());
      }
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
  server.insert("multihop_port", 1234);
  server.insert("socks5_name", "socks5_name");

  QJsonArray servers;
  servers.append(server);

  QJsonObject city;
  city.insert("code", "serverCityCode");
  city.insert("name", "serverCityName");
  city.insert("latitude", 12.34);
  city.insert("longitude", 34.56);
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
    SettingsHolder settingsHolder;
    Localizer l;

    QList<QVariant> results = m.recommendedLocations(1);
    QCOMPARE(results.length(), 1);

    QVariant qv = results.first();
    QVERIFY(qv.canConvert<const ServerCity*>());

    const ServerCity* city = qv.value<const ServerCity*>();
    QVERIFY(city != nullptr);
    QCOMPARE(city->country(), "serverCountryCode");
    QCOMPARE(city->name(), "serverCityName");
    QCOMPARE(city->localizedName(), "serverCityName");  // Localized?
  }
}

// ServerData
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverDataBasic() {
  SettingsHolder settingsHolder;

  ServerData sd;
  sd.initialize();

  QSignalSpy spy(&sd, &ServerData::changed);

  QVERIFY(!sd.hasServerData());
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
    cityObj.insert("latitude", 12.34);
    cityObj.insert("longitude", 34.56);
    cityObj.insert("servers", QJsonArray());

    ServerCity city;
    QVERIFY(city.fromJson(cityObj, "serverCountryCode"));

    sd.update(country.code(), city.name());
    QCOMPARE(spy.count(), 1);

    QVERIFY(sd.hasServerData());
    QCOMPARE(sd.exitCountryCode(), "serverCountryCode");
    QCOMPARE(sd.exitCityName(), "serverCityName");
    QVERIFY(!sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "");
    QCOMPARE(sd.entryCityName(), "");
    QCOMPARE(sd.previousExitCountryCode(), "");
    QCOMPARE(sd.previousExitCityName(), "");

    {
      ServerData sd2;
      sd2.initialize();

      QVERIFY(sd2.fromSettings());
      QVERIFY(sd2.hasServerData());
      QCOMPARE(sd2.exitCountryCode(), "serverCountryCode");
      QCOMPARE(sd2.exitCityName(), "serverCityName");
      QVERIFY(!sd2.multihop());
      QCOMPARE(sd2.entryCountryCode(), "");
      QCOMPARE(sd2.entryCityName(), "");
      QCOMPARE(sd2.previousExitCountryCode(), "");
      QCOMPARE(sd2.previousExitCityName(), "");

      QCOMPARE(spy.count(), 1);
    }
  }

  sd.update("new Country Code", "new City");
  QCOMPARE(spy.count(), 2);

  QVERIFY(sd.hasServerData());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");
  QCOMPARE(sd.previousExitCountryCode(), "serverCountryCode");
  QCOMPARE(sd.previousExitCityName(), "serverCityName");

  sd.forget();
  QCOMPARE(spy.count(), 2);

  QVERIFY(!sd.hasServerData());
  QCOMPARE(sd.exitCountryCode(), "");
  QCOMPARE(sd.exitCityName(), "");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");
  QCOMPARE(sd.previousExitCountryCode(), "");
  QCOMPARE(sd.previousExitCityName(), "");

  QVERIFY(sd.fromSettings());
  QCOMPARE(spy.count(), 3);

  sd.update("new Country Code", "new City", "entry Country Code", "entry City");
  QVERIFY(sd.hasServerData());
  QCOMPARE(sd.exitCountryCode(), "new Country Code");
  QCOMPARE(sd.exitCityName(), "new City");
  QVERIFY(sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "entry Country Code");
  QCOMPARE(sd.entryCityName(), "entry City");
  QCOMPARE(sd.previousExitCountryCode(), "new Country Code");
  QCOMPARE(sd.previousExitCityName(), "new City");

  sd.forget();
  QCOMPARE(spy.count(), 4);

  QVERIFY(!sd.hasServerData());
  QCOMPARE(sd.exitCountryCode(), "");
  QCOMPARE(sd.exitCityName(), "");
  QVERIFY(!sd.multihop());
  QCOMPARE(sd.entryCountryCode(), "");
  QCOMPARE(sd.entryCityName(), "");
  QCOMPARE(sd.previousExitCountryCode(), "");
  QCOMPARE(sd.previousExitCityName(), "");
}

void TestModels::serverDataMigrate() {
  {
    SettingsHolder settingsHolder;
    settingsHolder.setCurrentServerCountryCodeDeprecated("foo");
    settingsHolder.setCurrentServerCityDeprecated("bar");

    ServerData sd;
    sd.initialize();

    QVERIFY(sd.fromSettings());

    QCOMPARE(sd.exitCountryCode(), "foo");
    QCOMPARE(sd.exitCityName(), "bar");
    QVERIFY(!sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "");
    QCOMPARE(sd.entryCityName(), "");

    QVERIFY(settingsHolder.hasServerData());
    QVERIFY(!settingsHolder.hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCityDeprecated());

    ServerData sd2;
    sd2.initialize();

    QVERIFY(sd2.fromSettings());

    QCOMPARE(sd2.exitCountryCode(), "foo");
    QCOMPARE(sd2.exitCityName(), "bar");
    QVERIFY(!sd2.multihop());
    QCOMPARE(sd2.entryCountryCode(), "");
    QCOMPARE(sd2.entryCityName(), "");

    QVERIFY(settingsHolder.hasServerData());
    QVERIFY(!settingsHolder.hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCityDeprecated());
  }

  {
    SettingsHolder settingsHolder;
    settingsHolder.setCurrentServerCountryCodeDeprecated("foo");
    settingsHolder.setCurrentServerCityDeprecated("bar");
    settingsHolder.setEntryServerCountryCodeDeprecated("aa");
    settingsHolder.setEntryServerCityDeprecated("bb");

    ServerData sd;
    sd.initialize();

    QVERIFY(sd.fromSettings());

    QCOMPARE(sd.exitCountryCode(), "foo");
    QCOMPARE(sd.exitCityName(), "bar");
    QVERIFY(sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "aa");
    QCOMPARE(sd.entryCityName(), "bb");

    QVERIFY(settingsHolder.hasServerData());
    QVERIFY(!settingsHolder.hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCityDeprecated());

    ServerData sd2;
    sd2.initialize();

    QVERIFY(sd2.fromSettings());

    QCOMPARE(sd2.exitCountryCode(), "foo");
    QCOMPARE(sd2.exitCityName(), "bar");
    QVERIFY(sd2.multihop());
    QCOMPARE(sd2.entryCountryCode(), "aa");
    QCOMPARE(sd2.entryCityName(), "bb");

    QVERIFY(settingsHolder.hasServerData());
    QVERIFY(!settingsHolder.hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder.hasEntryServerCityDeprecated());
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

// Location
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::locationBasic() {
  Location location;
  QVERIFY(!location.initialized());
  QCOMPARE(location.cityName(), "");
  QCOMPARE(location.countryCode(), "");
  QCOMPARE(location.subdivision(), "");
  QVERIFY(qIsNaN(location.latitude()));
  QVERIFY(qIsNaN(location.longitude()));
  QVERIFY(location.ipAddress().isNull());
}

void TestModels::locationFromJson_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("city");
  QTest::addColumn<QString>("country");
  QTest::addColumn<QString>("subdivision");
  QTest::addColumn<bool>("hasLatLong");

  QTest::newRow("null") << QByteArray("") << false << ""
                        << ""
                        << "" << false;
  QTest::newRow("invalid") << QByteArray("wow") << false << ""
                           << ""
                           << "" << false;
  QTest::newRow("array") << QByteArray("[]]") << false << ""
                         << ""
                         << "" << false;

  QJsonObject obj;
  QTest::newRow("empty") << QJsonDocument(obj).toJson() << false << ""
                         << ""
                         << "" << false;

  obj.insert("city", QJsonValue("Mordor"));
  obj.insert("country", QJsonValue("XX"));
  obj.insert("subdivision", QJsonValue("MTDOOM"));
  obj.insert("ip", QJsonValue("169.254.0.1"));
  QTest::newRow("okay no geoip")
      << QJsonDocument(obj).toJson() << true << "Mordor"
      << "XX"
      << "MTDOOM" << false;

  obj.insert("lat_long", QJsonValue("3.14159,-2.71828"));
  QTest::newRow("okay with geoip")
      << QJsonDocument(obj).toJson() << true << "Mordor"
      << "XX"
      << "MTDOOM" << true;
}

void TestModels::locationFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);
  QFETCH(QString, city);
  QFETCH(QString, country);
  QFETCH(QString, subdivision);
  QFETCH(bool, hasLatLong);

  Location location;
  QCOMPARE(location.fromJson(json), result);

  QCOMPARE(location.cityName(), city);
  QCOMPARE(location.countryCode(), country);
  QCOMPARE(location.subdivision(), subdivision);
  if (hasLatLong) {
    QVERIFY(!qIsNaN(location.latitude()));
    QVERIFY(!qIsNaN(location.longitude()));
  } else {
    QVERIFY(qIsNaN(location.latitude()));
    QVERIFY(qIsNaN(location.longitude()));
  }
}

static TestModels s_testModels;
