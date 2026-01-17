/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testmodels.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaType>

#include "constants.h"
#include "helper.h"
#include "localizer.h"
#include "models/apierror.h"
#include "models/device.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/location.h"
#include "models/recentconnections.h"
#include "models/recommendedlocationmodel.h"
#include "models/servercity.h"
#include "models/servercountry.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "models/user.h"
#include "settingsholder.h"

// Curve 25519 example keypair from RFC 7748: Section 6.1
constexpr const char* X25519_ALICE_PRIVATE =
    "dwdtCnMYpX08FsFyUbJmRd9ML4frwJkqsXf7pR25LCo=";
constexpr const char* X25519_ALICE_PUBLIC =
    "hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmo=";
constexpr const char* X25519_BOB_PRIVATE =
    "XasIfmJKikt54X+Lg4AO5m87sSkmGLb9HC+LJ/+I4Os=";
constexpr const char* X25519_BOB_PUBLIC =
    "3p7bfXt9wbTTW2HC7OQ1Nz+DQ8hbeGdNrfx+FG+IK08=";

void TestModels::cleanup() { SettingsHolder::testCleanup(); }

// ApiError
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::apiErrorBasic() {
  ApiError err;

  QCOMPARE(err.code(), 0);
  QCOMPARE(err.errnum(), 0);
  QCOMPARE(err.message(), "");
  QCOMPARE(err.error(), err.message());
}

void TestModels::apiErrorParse() {
  ApiError err;

  // Parsing should succeed.
  QFETCH(QByteArray, json);
  QFETCH(int, code);
  QFETCH(int, errnum);
  QFETCH(QString, message);
  QCOMPARE(err.fromJson(json), true);

  QCOMPARE(err.code(), code);
  QCOMPARE(err.errnum(), errnum);
  QCOMPARE(err.message(), message);
  QCOMPARE(err.error(), message);

  // Check the copy constructor.
  ApiError copy(err);
  QCOMPARE(copy.code(), code);
  QCOMPARE(copy.errnum(), errnum);
  QCOMPARE(copy.message(), message);
  QCOMPARE(copy.error(), message);

  // Check the assignment operator.
  ApiError assign = err;
  QCOMPARE(assign.code(), code);
  QCOMPARE(assign.errnum(), errnum);
  QCOMPARE(assign.message(), message);
  QCOMPARE(assign.error(), message);
}

void TestModels::apiErrorParse_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<int>("code");
  QTest::addColumn<int>("errnum");
  QTest::addColumn<QString>("message");

  QJsonObject obj;
  obj.insert("code", 401);
  obj.insert("errno", 123);
  obj.insert("error", "Hello World!");
  QTest::addRow("success") << QJsonDocument(obj).toJson() << 401 << 123
                           << "Hello World!";

  obj.insert("extra", "Something amazing!");
  QTest::addRow("extra keys")
      << QJsonDocument(obj).toJson() << 401 << 123 << "Hello World!";
}

void TestModels::apiErrorInvalid() {
  ApiError err;

  // Parsing should fail.
  QFETCH(QByteArray, json);
  QCOMPARE(err.fromJson(json), false);

  // Nothing should change.
  QCOMPARE(err.code(), 0);
  QCOMPARE(err.errnum(), 0);
  QCOMPARE(err.message(), "");
  QCOMPARE(err.error(), err.message());
}

void TestModels::apiErrorInvalid_data() {
  QTest::addColumn<QByteArray>("json");

  QTest::addRow("null") << QJsonDocument().toJson();
  QTest::addRow("empty") << QJsonDocument(QJsonObject()).toJson();

  QJsonObject obj;
  obj.insert("code", 401);
  obj.insert("errno", 123);
  obj.insert("error", "Hello World!");

  QTest::addRow("array") << QJsonDocument(QJsonArray({obj})).toJson();
  QTest::addRow("unclosed") << QJsonDocument(obj).toJson().replace('}', '{');
  QTest::addRow("extra junk") << QJsonDocument(obj).toJson().append("Hi There");

  QJsonObject badcode(obj);
  badcode.remove("code");
  QTest::addRow("code missing") << QJsonDocument(badcode).toJson();
  badcode.insert("code", QJsonArray());
  QTest::addRow("code invalid") << QJsonDocument(badcode).toJson();

  QJsonObject baderrno(obj);
  baderrno.remove("errno");
  QTest::addRow("errno missing") << QJsonDocument(baderrno).toJson();
  baderrno.insert("errno", QJsonObject());
  QTest::addRow("errno invalid") << QJsonDocument(baderrno).toJson();

  QJsonObject badmessage(obj);
  badmessage.remove("error");
  QTest::addRow("error missing") << QJsonDocument(badmessage).toJson();
  badmessage.insert("error", 666);
  QTest::addRow("error invalid") << QJsonDocument(badmessage).toJson();
}

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
  d.insert("pubkey", X25519_BOB_PUBLIC);
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.replace(0, d);
  obj.insert("devices", devices);
  QTest::addRow("good") << QJsonDocument(obj).toJson() << true << 1
                        << QVariant("deviceName") << QVariant(X25519_BOB_PUBLIC)
                        << QVariant(false)
                        << QVariant(QDateTime::fromString("2017-07-24T15:46:29",
                                                          Qt::ISODate));

  d.insert("name", Device::currentDeviceName());
  d.insert("unique_id", "43");
  d.insert("pubkey", X25519_ALICE_PUBLIC);
  d.insert("created_at", "2017-07-24T15:46:29");
  d.insert("ipv4_address", "deviceIpv4");
  d.insert("ipv6_address", "deviceIpv6");

  devices.append(d);
  obj.insert("devices", devices);
  QTest::addRow("good - 2 devices")
      << QJsonDocument(obj).toJson() << true << 2
      << QVariant(Device::currentDeviceName()) << QVariant(X25519_ALICE_PUBLIC)
      << QVariant(true)
      << QVariant(QDateTime::fromString("2017-07-24T15:46:29", Qt::ISODate));
}

void TestModels::deviceModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // fromJson
  {
    Keys keys;
    keys.storeKeys(X25519_ALICE_PRIVATE, X25519_ALICE_PUBLIC);

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
    SettingsHolder::instance()->setDevices(json);

    Keys keys;
    keys.storeKeys(X25519_ALICE_PRIVATE, X25519_ALICE_PUBLIC);

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
      keys.storeKeys(X25519_ALICE_PRIVATE, X25519_ALICE_PUBLIC);

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
  keys.storeKeys(X25519_ALICE_PRIVATE, X25519_ALICE_PUBLIC);

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

  // fromSettings should fail on a bogus private key
  {
    QCOMPARE(k.fromSettings(""), false);
    QCOMPARE(k.fromSettings("WOW"), false);
    QCOMPARE(k.fromSettings("base64"), false);
  }

  // fromSettings should compute the corresponding public key.
  {
    QCOMPARE(k.fromSettings(X25519_ALICE_PRIVATE), true);
    QCOMPARE(k.privateKey(), X25519_ALICE_PRIVATE);
    QCOMPARE(k.publicKey(), X25519_ALICE_PUBLIC);

    QCOMPARE(k.fromSettings(X25519_BOB_PRIVATE), true);
    QCOMPARE(k.privateKey(), X25519_BOB_PRIVATE);
    QCOMPARE(k.publicKey(), X25519_BOB_PUBLIC);
  }
}

// Recent Connections
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::recentConnectionBasic() {
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
  for (int i = 1; i < 2 * Constants::RECENT_CONNECTIONS_MAX_COUNT; ++i) {
    MozillaVPN::instance()->serverData()->changeServer(
        QString("%1").arg('a' + i), QString("%1").arg('b' + i));
  }

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 1);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's read from the settings again.
  rc->initialize();

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()), 1);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's add a few multi-hop entries. We cannot reach the max value.
  for (int i = 1; i < 2 * Constants::RECENT_CONNECTIONS_MAX_COUNT; ++i) {
    MozillaVPN::instance()->serverData()->changeServer(
        QString("%1").arg('a' + i), QString("%1").arg('b' + i),
        QString("%1").arg('c' + 1), QString("%1").arg('d' + 1));
  }

  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcMultiHop->isEmpty());

  // Let's read from the settings again.
  rc->initialize();
  QCOMPARE(rcSingleHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
  QVERIFY(!rcSingleHop->isEmpty());

  QCOMPARE(rcMultiHop->rowCount(QModelIndex()),
           Constants::RECENT_CONNECTIONS_MAX_COUNT);
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
  QFETCH(QStringList, data);
  SettingsHolder::instance()->setRecentConnectionsDeprecated(data);
  QVERIFY(SettingsHolder::instance()->hasRecentConnectionsDeprecated());

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

  QVERIFY(!SettingsHolder::instance()->hasRecentConnectionsDeprecated());
}

void TestModels::recentConnectionSaveAndRestore() {
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

// RecommendedLocations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::recommendedLocationsPick() {
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

  {
    Localizer l;

    QCOMPARE(MozillaVPN::instance()->serverCountryModel()->fromJson(json),
             true);

    auto results = RecommendedLocationModel::recommendedLocations(1);
    QCOMPARE(results.length(), 1);

    const ServerCity* city = results.first();
    QVERIFY(city != nullptr);
    QCOMPARE(city->country(), "serverCountryCode");
    QCOMPARE(city->name(), "serverCityName");
    QCOMPARE(city->localizedName(), "serverCityName");  // Localized?
  }
}

// ServerData
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestModels::serverDataBasic() {
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
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    settingsHolder->setCurrentServerCountryCodeDeprecated("foo");
    settingsHolder->setCurrentServerCityDeprecated("bar");

    ServerData sd;
    sd.initialize();

    QVERIFY(sd.fromSettings());

    QCOMPARE(sd.exitCountryCode(), "foo");
    QCOMPARE(sd.exitCityName(), "bar");
    QVERIFY(!sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "");
    QCOMPARE(sd.entryCityName(), "");

    QVERIFY(settingsHolder->hasServerData());
    QVERIFY(!settingsHolder->hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCityDeprecated());
    ServerData sd2;
    sd2.initialize();

    QVERIFY(sd2.fromSettings());

    QCOMPARE(sd2.exitCountryCode(), "foo");
    QCOMPARE(sd2.exitCityName(), "bar");
    QVERIFY(!sd2.multihop());
    QCOMPARE(sd2.entryCountryCode(), "");
    QCOMPARE(sd2.entryCityName(), "");

    QVERIFY(settingsHolder->hasServerData());
    QVERIFY(!settingsHolder->hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCityDeprecated());
  }

  {
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    settingsHolder->setCurrentServerCountryCodeDeprecated("foo");
    settingsHolder->setCurrentServerCityDeprecated("bar");
    settingsHolder->setEntryServerCountryCodeDeprecated("aa");
    settingsHolder->setEntryServerCityDeprecated("bb");

    ServerData sd;
    sd.initialize();

    QVERIFY(sd.fromSettings());

    QCOMPARE(sd.exitCountryCode(), "foo");
    QCOMPARE(sd.exitCityName(), "bar");
    QVERIFY(sd.multihop());
    QCOMPARE(sd.entryCountryCode(), "aa");
    QCOMPARE(sd.entryCityName(), "bb");

    QVERIFY(settingsHolder->hasServerData());
    QVERIFY(!settingsHolder->hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCityDeprecated());
    ServerData sd2;
    sd2.initialize();

    QVERIFY(sd2.fromSettings());

    QCOMPARE(sd2.exitCountryCode(), "foo");
    QCOMPARE(sd2.exitCityName(), "bar");
    QVERIFY(sd2.multihop());
    QCOMPARE(sd2.entryCountryCode(), "aa");
    QCOMPARE(sd2.entryCityName(), "bb");

    QVERIFY(settingsHolder->hasServerData());
    QVERIFY(!settingsHolder->hasCurrentServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasCurrentServerCityDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCountryCodeDeprecated());
    QVERIFY(!settingsHolder->hasEntryServerCityDeprecated());
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

void TestModels::locationDistance_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<double>("latitude");
  QTest::addColumn<double>("longitude");
  QTest::addColumn<double>("result");

  QJsonObject obj;
  obj.insert("city", QJsonValue("Mordor"));
  obj.insert("country", QJsonValue("XX"));
  obj.insert("subdivision", QJsonValue("MTDOOM"));
  obj.insert("ip", QJsonValue("169.254.0.1"));

  // Check for error handling if the coordinates are missing.
  QTest::newRow("Both invalid to zero distance")
      << QJsonDocument(obj).toJson() << qQNaN() << qQNaN() << 0.0;

  QTest::newRow("Model invalid to zero distance")
      << QJsonDocument(obj).toJson() << 123.456 << 55.555 << 0.0;

  obj.insert("lat_long", QJsonValue("123.456,55.555"));
  QTest::newRow("Args invalid to zero distance")
      << QJsonDocument(obj).toJson() << qQNaN() << qQNaN() << 0.0;

  // Test vectors are generated from: http://edwilliams.org/gccalc.htm
  // with coordinates scraped from google maps.
  // distances are in km, and computed using a spherical earth model.
  obj.insert("lat_long", QJsonValue("39.9033766,32.7627648"));
  QTest::newRow("From Ankara to Izmir")
      << QJsonDocument(obj).toJson() << 38.4178607 << 26.9396341 << 528.06;

  obj.insert("lat_long", QJsonValue("43.1666908,131.8834184"));
  QTest::newRow("From Vladivastok to Anchorage")
      << QJsonDocument(obj).toJson() << 61.1083688 << -150.000681 << 5313.05;

  obj.insert("lat_long", QJsonValue("1.3143269,103.5571562"));
  QTest::newRow("From Singapore to Ecuador")
      << QJsonDocument(obj).toJson() << -0.1862486 << -78.717632 << 19719.47;

  obj.insert("lat_long", QJsonValue("-77.8400829,166.64453"));
  QTest::newRow("From McMurdo Station to Bering Island")
      << QJsonDocument(obj).toJson() << 55.0218511 << 165.9355717 << 14763.70;

  obj.insert("lat_long", QJsonValue("90,0"));
  QTest::newRow("From North to South poles")
      << QJsonDocument(obj).toJson() << -90.0 << 123.456 << 20001.6;
}

void TestModels::locationDistance() {
  QFETCH(QByteArray, json);
  QFETCH(double, latitude);
  QFETCH(double, longitude);
  QFETCH(double, result);

  // A word about the scale. The Location class computes distance in radians,
  // but the earth isn't actually round, it's squashed a little bit. This figure
  // here converts radians into kilometers using a spherical earth model with
  // 1 degree minute equal to 1 nautical mile.
  constexpr double scale = (1.852 * 360.0 * 60.0) / (M_PI * 2.0);
  constexpr double epsilon = 0.1;  // Maximum tolerated floating point error.
  double distance;
  Location location;
  QVERIFY(location.fromJson(json));

  // Check that we get the expected distance.
  distance = location.distance(latitude, longitude) * scale;
  QVERIFY(qFabs<double>(distance - result) < epsilon);

  // Check that we get the same result when called between two QObjects
  // with latitude/longitude coordinates.
  QObject obj;
  obj.setProperty("latitude", QVariant(latitude));
  obj.setProperty("longitude", QVariant(longitude));
  distance = Location::distance(&location, &obj) * scale;
  QVERIFY(qFabs<double>(distance - result) < epsilon);

  // And the same result again if we transpose the arguments.
  distance = Location::distance(&obj, &location) * scale;
  QVERIFY(qFabs<double>(distance - result) < epsilon);
}

static TestModels s_testModels;
