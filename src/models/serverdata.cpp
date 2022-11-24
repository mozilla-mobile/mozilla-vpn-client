/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverdata.h"
#include "leakdetector.h"
#include "logger.h"
#include "servercountrymodel.h"
#include "serveri18n.h"
#include "settingsholder.h"

#include <QJsonDocument>
#include <QJsonObject>

constexpr const char* EXIT_COUNTRY_CODE = "exit_country_code";
constexpr const char* EXIT_CITY_NAME = "exit_city_name";
constexpr const char* ENTER_COUNTRY_CODE = "enter_country_code";
constexpr const char* ENTER_CITY_NAME = "enter_city_name";

namespace {
Logger logger(LOG_MODEL, "ServerData");
}

ServerData::ServerData() { MVPN_COUNT_CTOR(ServerData); }

ServerData::~ServerData() { MVPN_COUNT_DTOR(ServerData); }

bool ServerData::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // Let's migrate data from a pre v2.13.
  if (settingsHolder->hasCurrentServerCountryCodeDeprecated() &&
      settingsHolder->hasCurrentServerCityDeprecated()) {
    initializeInternal(settingsHolder->currentServerCountryCodeDeprecated(),
                       settingsHolder->currentServerCityDeprecated(),
                       settingsHolder->entryServerCountryCodeDeprecated(),
                       settingsHolder->entryServerCityDeprecated());

    writeSettings();
    Q_ASSERT(settingsHolder->hasServerData());

    settingsHolder->removeCurrentServerCountryCodeDeprecated();
    settingsHolder->removeCurrentServerCityDeprecated();
    settingsHolder->removeEntryServerCountryCodeDeprecated();
    settingsHolder->removeEntryServerCityDeprecated();

    return true;
  }

  if (!settingsHolder->hasServerData()) {
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(settingsHolder->serverData());
  if (!json.isObject()) {
    return false;
  }

  QJsonObject obj = json.object();

  initializeInternal(
      obj[EXIT_COUNTRY_CODE].toString(), obj[EXIT_CITY_NAME].toString(),
      obj[ENTER_COUNTRY_CODE].toString(), obj[ENTER_CITY_NAME].toString());

  logger.debug() << toString();
  return true;
}

void ServerData::writeSettings() {
  QJsonObject obj;
  obj[EXIT_COUNTRY_CODE] = m_exitCountryCode;
  obj[EXIT_CITY_NAME] = m_exitCityName;

  if (multihop()) {
    obj[ENTER_COUNTRY_CODE] = m_entryCountryCode;
    obj[ENTER_CITY_NAME] = m_entryCityName;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setServerData(QJsonDocument(obj).toJson());
}

void ServerData::update(const QString& countryCode, const QString& cityName,
                        const QString& entryCountryCode,
                        const QString& entryCityName) {
  initializeInternal(countryCode, cityName, entryCountryCode, entryCityName);
  emit changed();
}

void ServerData::initializeInternal(const QString& exitCountryCode,
                                    const QString& exitCityName,
                                    const QString& entryCountryCode,
                                    const QString& entryCityName) {
  m_initialized = true;
  m_exitCountryCode = exitCountryCode;
  m_exitCityName = exitCityName;
  m_entryCountryCode = entryCountryCode;
  m_entryCityName = entryCityName;
}

QString ServerData::localizedCityName() const {
  return ServerI18N::translateCityName(m_exitCountryCode, m_exitCityName);
}

QString ServerData::localizedEntryCity() const {
  return ServerI18N::translateCityName(m_entryCountryCode, m_entryCityName);
}

QString ServerData::toString() const {
  if (!m_initialized) {
    return QString();
  }

  QString result = "";
  if (multihop()) {
    result += m_entryCityName + ", " + m_entryCountryCode + " -> ";
  }

  result += m_exitCityName + ", " + m_exitCountryCode;
  return result;
}
