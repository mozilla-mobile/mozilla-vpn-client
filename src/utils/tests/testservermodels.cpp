/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testservermodels.h"

#include <QtTest/QtTest>

#include "models/servercity.h"
#include "models/servercountry.h"
#include "models/servercountrymodel.h"

// Server
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestServerModels::serverBasic() {
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

void TestServerModels::serverFromJson_data() {
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

void TestServerModels::serverFromJson() {
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
  QVERIFY(ports.length() >= 1);
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
}

void TestServerModels::serverWeightChooser() {
  QList<Server> list;
  list.append(Server());

  const Server& s = Server::weightChooser(list);
  QCOMPARE(&s, &list[0]);
}

// ServerCity
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestServerModels::serverCityBasic() {
  ServerCity sc;
  QCOMPARE(sc.name(), "");
  QCOMPARE(sc.code(), "");
  QVERIFY(sc.servers().isEmpty());
}

void TestServerModels::serverCityFromJson_data() {
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

void TestServerModels::serverCityFromJson() {
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
}

// ServerCountry
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestServerModels::serverCountryBasic() {
  ServerCountry sc;
  QCOMPARE(sc.name(), "");
  QCOMPARE(sc.code(), "");
  QVERIFY(sc.cities().isEmpty());
}

void TestServerModels::serverCountryFromJson_data() {
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

void TestServerModels::serverCountryFromJson() {
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
}

// ServerCountryModel
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestServerModels::serverCountryModelBasic() {
  ServerCountryModel scm;
  QVERIFY(!scm.initialized());

  QVERIFY(!scm.fromJson(QByteArray()));

  QHash<int, QByteArray> rn = scm.roleNames();
  QCOMPARE(rn.count(), 4);
  QCOMPARE(rn[ServerCountryModel::NameRole], "name");
  QCOMPARE(rn[ServerCountryModel::LocalizedNameRole], "localizedName");
  QCOMPARE(rn[ServerCountryModel::CodeRole], "code");
  QCOMPARE(rn[ServerCountryModel::CitiesRole], "cities");

  QCOMPARE(scm.rowCount(QModelIndex()), 0);
  QCOMPARE(scm.data(QModelIndex(), ServerCountryModel::NameRole), QVariant());
}

void TestServerModels::serverCountryModelFromJson_data() {
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

void TestServerModels::serverCountryModelFromJson() {
  QFETCH(QByteArray, json);
  QFETCH(bool, result);

  // from json
  {
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
        QVERIFY(cities.typeId() == QMetaType::QVariantList);
        QVariant cityData = m.data(index, ServerCountryModel::CitiesRole);
        QCOMPARE(cityData.typeId(), QMetaType::QVariantList);
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
}
