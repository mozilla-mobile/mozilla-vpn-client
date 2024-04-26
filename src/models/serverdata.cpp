/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverdata.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "mozillavpn.h"
#include "servercountrymodel.h"
#include "serverlatency.h"
#include "settingsholder.h"

constexpr const char* EXIT_COUNTRY_CODE = "exit_country_code";
constexpr const char* EXIT_COUNTRY_NAME = "exit_country_name";
constexpr const char* EXIT_CITY_NAME = "exit_city_name";
constexpr const char* ENTER_COUNTRY_CODE = "enter_country_code";
constexpr const char* ENTER_COUNTRY_NAME = "enter_country_name";
constexpr const char* ENTER_CITY_NAME = "enter_city_name";

namespace {
Logger logger("ServerData");
}

ServerData::ServerData() { MZ_COUNT_CTOR(ServerData); }

ServerData::ServerData(const ServerData& other) {
  MZ_COUNT_CTOR(ServerData);
  *this = other;
}

ServerData::~ServerData() { MZ_COUNT_DTOR(ServerData); }

ServerData& ServerData::operator=(const ServerData& other) {
  m_initialized = other.m_initialized;
  m_exitCountryCode = other.m_exitCountryCode;
  m_exitCityName = other.m_exitCityName;
  m_entryCountryCode = other.m_entryCountryCode;
  m_entryCityName = other.m_entryCityName;
  m_previousExitCountryCode = other.m_previousExitCountryCode;
  m_previousExitCityName = other.m_previousExitCityName;
  m_exitServerPublicKey = other.m_exitServerPublicKey;
  m_entryServerPublicKey = other.m_entryServerPublicKey;

  return *this;
}

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
  m_previousExitCountryName =
      MozillaVPN::instance()->serverCountryModel()->countryName(
          exitCountryCode);
  m_previousExitCityName = m_exitCityName;

  QJsonObject obj;
  obj[EXIT_COUNTRY_CODE] = exitCountryCode;
  obj[EXIT_COUNTRY_NAME] =
      MozillaVPN::instance()->serverCountryModel()->countryName(
          exitCountryCode);
  obj[EXIT_CITY_NAME] = exitCityName;

  obj[ENTER_COUNTRY_CODE] = entryCountryCode;
  obj[ENTER_COUNTRY_NAME] =
      MozillaVPN::instance()->serverCountryModel()->countryName(
          entryCountryCode);
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
  // If obj[EXIT_COUNTRY_NAME] does not exist then we need to extract the
  // country name from the country code
  if (!obj.contains(EXIT_COUNTRY_NAME)) {
    m_exitCountryName =
        MozillaVPN::instance()->serverCountryModel()->countryName(
            m_exitCountryCode);
  } else {
    m_exitCountryName = obj[EXIT_COUNTRY_NAME].toString();
  }

  m_entryCountryCode = obj[ENTER_COUNTRY_CODE].toString();
  m_entryCityName = obj[ENTER_CITY_NAME].toString();
  // If obj[ENTER_COUNTRY_NAME] does not exist then we need to extract the
  // country name from the country code
  if (!obj.contains(ENTER_COUNTRY_NAME)) {
    m_entryCountryName =
        MozillaVPN::instance()->serverCountryModel()->countryName(
            m_entryCountryCode);
  } else {
    m_entryCountryName = obj[ENTER_COUNTRY_NAME].toString();
  }

  emit changed();
  emit retranslationNeeded();
  return true;
}

QString ServerData::localizedExitCityName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCityName(m_exitCityName);
}

QString ServerData::localizedEntryCityName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCityName(m_entryCityName);
}

QString ServerData::localizedPreviousExitCountryName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCityName(
      m_previousExitCountryName);
}

QString ServerData::localizedPreviousExitCityName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCityName(m_previousExitCityName);
}

QString ServerData::localizedEntryCountryName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCountryName(m_entryCountryCode);
}

QString ServerData::localizedExitCountryName() const {
  Q_ASSERT(m_initialized);
  return Localizer::instance()->getTranslatedCountryName(m_exitCountryCode);
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

  if (entryCountryCode.isEmpty() && entryCityName.isEmpty()) {
    logger.info() << "Single hop server change";
    mozilla::glean::sample::user_changed_endpoint_geo.record(
        mozilla::glean::sample::UserChangedEndpointGeoExtra{
            ._server = "SingleHop",
        });
  } else {
    logger.info() << "Multihop server change";
    // If exit changed, or changing from single hop to multihop
    // (could have same exit for single and multihop, and we'd want to record
    // metric)
    if (m_exitCountryCode != countryCode || m_exitCityName != cityName ||
        (m_entryCountryCode.isEmpty() && m_entryCityName.isEmpty())) {
      logger.info() << "Multihop exit server change";
      mozilla::glean::sample::user_changed_endpoint_geo.record(
          mozilla::glean::sample::UserChangedEndpointGeoExtra{
              ._server = "MultiHopExit",
          });
    }

    // Cannot use an if/else, as it's possible both exit and entry changed
    if (m_entryCountryCode != entryCountryCode ||
        m_entryCityName != entryCityName) {
      logger.info() << "Multihop entry server change";
      mozilla::glean::sample::user_changed_endpoint_geo.record(
          mozilla::glean::sample::UserChangedEndpointGeoExtra{
              ._server = "MultiHopEntry",
          });
    }
  }

  update(countryCode, cityName, entryCountryCode, entryCityName);
}

void ServerData::forget() {
  Q_ASSERT(m_initialized);

  m_exitCountryCode.clear();
  m_exitCityName.clear();
  m_exitCountryName.clear();
  m_entryCountryCode.clear();
  m_entryCityName.clear();
  m_entryCountryName.clear();
  m_previousExitCountryCode.clear();
  m_previousExitCountryName.clear();
  m_previousExitCityName.clear();
}

// static
QList<Server> ServerData::getServerList(const QString& countryCode,
                                        const QString& cityName) {
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  ServerLatency* serverLatency = MozillaVPN::instance()->serverLatency();
  const ServerCity& city = scm->findCity(countryCode, cityName);
  QList<Server> results;
  qint64 now = QDateTime::currentSecsSinceEpoch();

  for (const QString& pubkey : city.servers()) {
    const Server& server = scm->server(pubkey);
    if (server.initialized() && (serverLatency->getCooldown(pubkey) <= now)) {
      results.append(server);
    }
  }

  return results;
}

const QList<Server> ServerData::exitServers() const {
  return getServerList(m_exitCountryCode, m_exitCityName);
}

const QList<Server> ServerData::entryServers() const {
  if (!multihop()) {
    return exitServers();
  }

  return getServerList(m_entryCountryCode, m_entryCityName);
}

void ServerData::setEntryServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set entry-server public key:" << logger.keys(publicKey);
  m_entryServerPublicKey = publicKey;
}

void ServerData::setExitServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set exit-server public key:" << logger.keys(publicKey);
  m_exitServerPublicKey = publicKey;
}

const Server ServerData::backupServer(const QString& currentPublicKey) const {
  const QList<Server> serverList = exitServers();
  foreach (auto item, serverList) {
    if (item.publicKey() != currentPublicKey) {
      return item;
    }
  }
  return Server();
}
