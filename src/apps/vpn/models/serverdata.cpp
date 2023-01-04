/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverdata.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "servercountrymodel.h"
#include "serveri18n.h"
#include "settingsholder.h"

constexpr const char* EXIT_COUNTRY_CODE = "exit_country_code";
constexpr const char* EXIT_CITY_NAME = "exit_city_name";
constexpr const char* ENTER_COUNTRY_CODE = "enter_country_code";
constexpr const char* ENTER_CITY_NAME = "enter_city_name";

namespace {
Logger logger("ServerData");

QList<Server> filterServerList(const QList<Server>& servers) {
  QList<Server> results;
  qint64 now = QDateTime::currentSecsSinceEpoch();

  for (const Server& server : servers) {
    if (server.cooldownTimeout() <= now) {
      results.append(server);
    }
  }

  return results;
}

}  // namespace

ServerData::ServerData() { MZ_COUNT_CTOR(ServerData); }

ServerData::~ServerData() { MZ_COUNT_DTOR(ServerData); }

void ServerData::initialize() {
  m_initialized = true;

  connect(SettingsHolder::instance(), &SettingsHolder::serverDataChanged, this,
          &ServerData::settingsChanged);
}

bool ServerData::fromSettings() {
  Q_ASSERT(m_initialized);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // Let's migrate data from a pre v2.13.
  if (settingsHolder->hasCurrentServerCountryCodeDeprecated() &&
      settingsHolder->hasCurrentServerCityDeprecated()) {
    update(settingsHolder->currentServerCountryCodeDeprecated(),
           settingsHolder->currentServerCityDeprecated(),
           settingsHolder->entryServerCountryCodeDeprecated(),
           settingsHolder->entryServerCityDeprecated());

    Q_ASSERT(settingsHolder->hasServerData());

    settingsHolder->removeCurrentServerCountryCodeDeprecated();
    settingsHolder->removeCurrentServerCityDeprecated();
    settingsHolder->removeEntryServerCountryCodeDeprecated();
    settingsHolder->removeEntryServerCityDeprecated();

    return true;
  }

  return settingsChanged();
}

void ServerData::update(const QString& exitCountryCode,
                        const QString& exitCityName,
                        const QString& entryCountryCode,
                        const QString& entryCityName) {
  Q_ASSERT(m_initialized);

  m_previousExitCountryCode = m_exitCountryCode;
  m_previousExitCityName = m_exitCityName;

  QJsonObject obj;
  obj[EXIT_COUNTRY_CODE] = exitCountryCode;
  obj[EXIT_CITY_NAME] = exitCityName;

  obj[ENTER_COUNTRY_CODE] = entryCountryCode;
  obj[ENTER_CITY_NAME] = entryCityName;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setServerData(
      QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

bool ServerData::settingsChanged() {
  Q_ASSERT(m_initialized);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (!settingsHolder->hasServerData()) {
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(settingsHolder->serverData());
  if (!json.isObject()) {
    return false;
  }

  QJsonObject obj = json.object();

  m_exitCountryCode = obj[EXIT_COUNTRY_CODE].toString();
  m_exitCityName = obj[EXIT_CITY_NAME].toString();
  m_entryCountryCode = obj[ENTER_COUNTRY_CODE].toString();
  m_entryCityName = obj[ENTER_CITY_NAME].toString();

  emit changed();
  return true;
}

QString ServerData::localizedExitCityName() const {
  Q_ASSERT(m_initialized);
  return ServerI18N::translateCityName(m_exitCountryCode, m_exitCityName);
}

QString ServerData::localizedEntryCityName() const {
  Q_ASSERT(m_initialized);
  return ServerI18N::translateCityName(m_entryCountryCode, m_entryCityName);
}

QString ServerData::localizedPreviousExitCityName() const {
  Q_ASSERT(m_initialized);
  return ServerI18N::translateCityName(m_previousExitCountryCode,
                                       m_previousExitCityName);
}

void ServerData::changeServer(const QString& countryCode,
                              const QString& cityName,
                              const QString& entryCountryCode,
                              const QString& entryCityName) {
  if (m_exitCountryCode == countryCode && m_exitCityName == cityName &&
      m_entryCountryCode == entryCountryCode &&
      m_entryCityName == entryCityName) {
    logger.debug() << "No server change needed";
    return;
  }

  update(countryCode, cityName, entryCountryCode, entryCityName);
}

void ServerData::forget() {
  Q_ASSERT(m_initialized);

  m_exitCountryCode.clear();
  m_exitCityName.clear();
  m_entryCountryCode.clear();
  m_entryCityName.clear();
  m_previousExitCountryCode.clear();
  m_previousExitCityName.clear();
}

const QList<Server> ServerData::exitServers() const {
  return filterServerList(MozillaVPN::instance()->serverCountryModel()->servers(
      m_exitCountryCode, m_exitCityName));
}

const QList<Server> ServerData::entryServers() const {
  if (!multihop()) {
    return exitServers();
  }

  return filterServerList(MozillaVPN::instance()->serverCountryModel()->servers(
      m_entryCountryCode, m_entryCityName));
}

void ServerData::setEntryServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set entry-server public key:" << logger.keys(publicKey);
  m_entryServerPublicKey = publicKey;
}

void ServerData::setExitServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set exit-server public key:" << logger.keys(publicKey);
  m_exitServerPublicKey = publicKey;
}
