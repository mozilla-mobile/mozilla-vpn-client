
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androiddatamigration.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/accountandservers/taskaccountandservers.h"

#include "androidsharedprefs.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace {
Logger logger(LOG_ANDROID, "AndroidDataMigration");
#ifdef QT_DEBUG
const QString MIGRATION_FILE = "org.mozilla.firefox.vpn.debug_preferences.xml";
#else
const QString MIGRATION_FILE = "org.mozilla.firefox.vpn_preferences.xml";
#endif
void importDeviceInfo() {
  QVariant prefValue =
      AndroidSharedPrefs::GetValue(MIGRATION_FILE, "pref_current_device");
  if (!prefValue.isValid()) {
    logger.log() << "Failed to read pref_current_device - abort";
    return;
  }

  QJsonDocument prefValueJSON =
      QJsonDocument::fromJson(prefValue.toByteArray());
  auto deviceInfo = prefValueJSON.object();
  QString privateKey = deviceInfo["privateKeyBase64"].toString();
  QString pubKey = deviceInfo["device"].toObject()["pubkey"].toString();
  QString name = deviceInfo["device"].toObject()["name"].toString();
  if (!privateKey.isEmpty()) {
    MozillaVPN::instance()->deviceAdded(name, pubKey, privateKey);
  }
  logger.log() << "pref_current_device value was migrated";
}

void importUserInfo() {
  // Import User Information
  QVariant prefValue =
      AndroidSharedPrefs::GetValue(MIGRATION_FILE, "user_info");
  if (!prefValue.isValid()) {
    logger.log() << "Failed to read user_info";
    return;
  }
  QJsonDocument prefValueJSON =
      QJsonDocument::fromJson(prefValue.toByteArray());
  // We're having here {latestUpdateTime: <int>, user: <fxaUser> }
  auto obj = prefValueJSON.object();
  QJsonDocument userDoc(obj["user"].toObject());
  MozillaVPN::instance()->accountChecked(userDoc.toJson());
  logger.log() << "user_info value was imported";
}

void importLoginToken() {
  QVariant loginToken =
      AndroidSharedPrefs::GetValue(MIGRATION_FILE, "auth_token");
  if (!loginToken.isValid()) {
    logger.log() << "Failed to read auth_token";
    return;
  }
  MozillaVPN::instance()->setToken(loginToken.toString());

  logger.log() << "auth_token value was imported";
}

void importServerList() {
  QVariant prefValue =
      AndroidSharedPrefs::GetValue(MIGRATION_FILE, "pref_servers");
  if (!prefValue.isValid()) {
    logger.log() << "Failed to read pref_servers - exiting";
    return;
  }
  // We're having here {latestUpdateTime: <int>, servers: [<server>] }
  // Server:
  /* {
         "city": {"code": "mel","latitude": -37.815018,"longitude":
     144.946014,"name": "Melbourne"}, "country": {"code": "au","name":
     "Australia"}, "server": { "hostname": "au3-wireguard",
             "include_in_country": true,
             "ipv4_addr_in": "103.231.88.2",
             "ipv4_gateway": "10.64.0.1",
             "ipv6_gateway": "fc00:bbbb:bbbb:bb01::1",
             "port_ranges": [[53,53],[4000,33433],[33565,51820],[52000,60000]],
             "public_key": "Rzh64qPcg8W8klJq0H4EZdVCH7iaPuQ9falc99GTgRA\u003d",
             "weight": 3
         }
   */

  // transform this to {countries: [{name,code,city:[
  // {name,code,servers:[server]} ]} ]}
  QJsonDocument serverListDoc =
      QJsonDocument::fromJson(prefValue.toByteArray());
  QJsonArray serverList = serverListDoc.object()["servers"].toArray();

  QJsonObject out;
  QJsonArray countriesArr;

  // Cluster servers by country
  QMap<QString, QList<QJsonObject>> serversInCountry;
  foreach (auto server, serverList) {
    auto countryCode =
        server.toObject()["country"].toObject()["code"].toString();
    serversInCountry[countryCode].append(server.toObject());
  }

  foreach (auto countryCode, serversInCountry.keys()) {
    auto serversInSameCountry = serversInCountry[countryCode];

    QJsonObject countryObj;
    countryObj["name"] = serverList.first()["country"].toObject()["name"];
    countryObj["code"] = countryCode;

    QJsonArray citiesArr;
    // Cluster servers by city
    QMap<QString, QList<QJsonObject>> serversInCity;
    foreach (auto server, serversInSameCountry) {
      auto cityCode = server["city"].toObject()["code"].toString();
      serversInCity[cityCode].append(server);
    }
    foreach (auto cityCode, serversInCity.keys()) {
      auto serversInSameCity = serversInCity[cityCode];
      QJsonObject cityObj;
      cityObj["code"] = cityCode;
      cityObj["name"] = serversInSameCity.first()["city"].toObject()["name"];
      QJsonArray serversArray;
      foreach (auto s, serversInSameCity) { serversArray.append(s["server"]); }
      cityObj["servers"] = serversArray;
      citiesArr.append(cityObj);
    }

    countryObj["cities"] = citiesArr;
    countriesArr.append(countryObj);
  }
  out["countries"] = countriesArr;
  QJsonDocument outDoc(out);
  logger.log() << "Import JSON \n" << QString(outDoc.toJson());

  bool ok = MozillaVPN::instance()->setServerList(outDoc.toJson());
  if (!ok) {
    logger.log() << "pref_servers value was rejected";
    return;
  }
  logger.log() << "pref_servers value was imported";
}

}  // namespace

// static
void AndroidDataMigration::migrate() {
  logger.log() << "Android Data Migration -- Start";

  auto files = AndroidSharedPrefs::GetPrefFiles();
  if (!files.contains(MIGRATION_FILE)) {
    logger.log() << "Migration" << MIGRATION_FILE
                 << "was not file found - skip";
    logger.log() << "Migration files: " << files;
    return;
  };
  importDeviceInfo();
  importUserInfo();
  importLoginToken();
  importServerList();
}
