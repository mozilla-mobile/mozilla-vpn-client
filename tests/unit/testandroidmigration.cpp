/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testandroidmigration.h"
#include "../../src/platforms/android/androiddatamigration.h"
#include "helper.h"

void TestAndroidMigration::deviceInfo_data() {
  QTest::addColumn<QByteArray>("json");
  QTest::addColumn<QString>("privateKey");
  QTest::addColumn<QString>("publicKey");
  QTest::addColumn<QString>("name");

  QTest::addRow("null") << QByteArray() << ""
                        << ""
                        << "";
  QTest::addRow("array") << QByteArray("[]") << ""
                         << ""
                         << "";
  QTest::addRow("empty") << QByteArray("{}") << ""
                         << ""
                         << "";

  QJsonObject obj;
  obj.insert("privateKeyBase64", "a");

  QJsonObject device;
  device.insert("pubkey", "b");
  device.insert("name", "c");
  obj.insert("device", device);

  QTest::addRow("ok") << QJsonDocument(obj).toJson() << "a"
                      << "b"
                      << "c";
}

void TestAndroidMigration::deviceInfo() {
  QFETCH(QByteArray, json);
  QFETCH(QString, privateKey);
  QFETCH(QString, publicKey);
  QFETCH(QString, name);

  QString privateKeyValue;
  QString publicKeyValue;
  QString nameValue;

  AndroidDataMigration::importDeviceInfoInternal(json, privateKeyValue,
                                                 publicKeyValue, nameValue);
  QCOMPARE(privateKey, privateKeyValue);
  QCOMPARE(publicKey, publicKeyValue);
  QCOMPARE(name, nameValue);
}

void TestAndroidMigration::userInfo_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<QByteArray>("output");

  QTest::addRow("null") << QByteArray() << QByteArray();
  QTest::addRow("array") << QByteArray("[]") << QByteArray();
  QTest::addRow("empty") << QByteArray("{}") << QByteArray();

  QJsonObject obj;
  obj.insert("user", "a");
  QTest::addRow("invalid") << QJsonDocument(obj).toJson() << QByteArray();

  obj.insert("user", QJsonObject());
  QTest::addRow("ok") << QJsonDocument(obj).toJson() << QByteArray("{}");
}

void TestAndroidMigration::userInfo() {
  QFETCH(QByteArray, input);
  QFETCH(QByteArray, output);

  QCOMPARE(AndroidDataMigration::importUserInfoInternal(input), output);
}

void TestAndroidMigration::serverList_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<QByteArray>("output");

  QTest::addRow("null") << QByteArray() << QByteArray();
  QTest::addRow("array") << QByteArray("[]") << QByteArray();
  QTest::addRow("empty") << QByteArray("{}") << QByteArray();

  QJsonObject obj;
  obj.insert("servers", 42);
  QTest::addRow("invalid servers")
      << QJsonDocument(obj).toJson() << QByteArray();

  obj.insert("servers", QJsonArray());
  QTest::addRow("empty servers") << QJsonDocument(obj).toJson() << QByteArray();

  QJsonObject item;
  item["country"] = 42;
  item["city"] = 42;
  item["server"] = 42;

  QJsonArray servers;
  servers.append(item);
  obj.insert("servers", servers);

  QTest::addRow("invalid server")
      << QJsonDocument(obj).toJson() << QByteArray();

  item["country"] = QJsonObject();
  item["city"] = QJsonObject();
  item["server"] = QJsonObject();

  servers.replace(0, item);
  obj.insert("servers", servers);

  QTest::addRow("invalid server 2")
      << QJsonDocument(obj).toJson() << QByteArray();

  QJsonObject country;
  country["code"] = "BD";
  // no name here.

  item["country"] = country;

  servers.replace(0, item);
  obj.insert("servers", servers);

  QTest::addRow("invalid - no named country")
      << QJsonDocument(obj).toJson() << QByteArray();

  country["name"] = "Barad-dûr";

  item["country"] = country;

  servers.replace(0, item);
  obj.insert("servers", servers);

  QTest::addRow("invalid - good country - bad city")
      << QJsonDocument(obj).toJson() << QByteArray();

  QJsonObject city;
  city["code"] = "MD";
  // no name here.

  item["city"] = city;

  servers.replace(0, item);
  obj.insert("servers", servers);

  QTest::addRow("invalid - good country - no named city")
      << QJsonDocument(obj).toJson() << QByteArray();

  city["name"] = "Mount Doom";

  item["country"] = country;
  item["city"] = city;

  servers.replace(0, item);
  obj.insert("servers", servers);

  QJsonArray resultServers;
  resultServers.append(QJsonObject());

  QJsonObject resultCity;
  resultCity["name"] = city["name"];
  resultCity["code"] = city["code"];
  resultCity["servers"] = resultServers;

  QJsonArray resultCities;
  resultCities.append(resultCity);

  QJsonObject resultCountry;
  resultCountry["name"] = country["name"];
  resultCountry["code"] = country["code"];
  resultCountry["cities"] = resultCities;

  QJsonArray resultCountries;
  resultCountries.append(resultCountry);

  QJsonObject result;
  result["countries"] = resultCountries;

  QTest::addRow("invalid - good country - good city")
      << QJsonDocument(obj).toJson()
      << QJsonDocument(result).toJson(QJsonDocument::Compact);

  QJsonObject city2;
  city2["code"] = "DG";
  city2["name"] = "Dol Guldur";

  QJsonObject item2;
  item2["country"] = country;
  item2["city"] = city2;
  item2["server"] = QJsonObject();

  servers.append(item2);
  obj.insert("servers", servers);

  QJsonObject resultCity2;
  resultCity2["name"] = city2["name"];
  resultCity2["code"] = city2["code"];
  resultCity2["servers"] = resultServers;

  resultCities.replace(0, resultCity2);
  resultCities.append(resultCity);

  resultCountry["cities"] = resultCities;
  resultCountries.replace(0, resultCountry);
  result["countries"] = resultCountries;

  QTest::addRow("invalid - good country - two good cities")
      << QJsonDocument(obj).toJson()
      << QJsonDocument(result).toJson(QJsonDocument::Compact);

  QJsonObject country2;
  country2["code"] = "CO";
  country2["name"] = "Coruscant";

  QJsonObject city3;
  city3["code"] = "SD";
  city3["name"] = "Senate District";

  QJsonObject item3;
  item3["country"] = country2;
  item3["city"] = city3;
  item3["server"] = QJsonObject();

  servers.append(item3);
  obj.insert("servers", servers);

  QJsonObject resultCity3;
  resultCity3["name"] = city3["name"];
  resultCity3["code"] = city3["code"];
  resultCity3["servers"] = resultServers;

  QJsonArray resultCities3;
  resultCities3.append(resultCity3);

  QJsonObject resultCountry2;
  resultCountry2["name"] = country2["name"];
  resultCountry2["code"] = country2["code"];
  resultCountry2["cities"] = resultCities3;

  resultCountries.append(resultCountry2);
  result["countries"] = resultCountries;

  QTest::addRow("invalid - two good countries - three good cities")
      << QJsonDocument(obj).toJson()
      << QJsonDocument(result).toJson(QJsonDocument::Compact);
}

void TestAndroidMigration::serverList() {
  QFETCH(QByteArray, input);
  QFETCH(QByteArray, output);

  QCOMPARE(AndroidDataMigration::importServerListInternal(input), output);
}

static TestAndroidMigration s_testAndroidMigration;
