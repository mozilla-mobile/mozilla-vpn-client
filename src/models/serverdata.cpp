/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverdata.h"
#include "leakdetector.h"
#include "logger.h"
#include "servercountrymodel.h"
#include "serveri18n.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_MODEL, "ServerData");
}

ServerData::ServerData() { MVPN_COUNT_CTOR(ServerData); }

ServerData::~ServerData() { MVPN_COUNT_DTOR(ServerData); }

bool ServerData::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (!settingsHolder->hasCurrentServerCountry() ||
      !settingsHolder->hasCurrentServerCity() ||
      !settingsHolder->hasCurrentServerCountryCode()) {
    return false;
  }

  initializeInternal(settingsHolder->currentServerCountryCode(),
                     settingsHolder->currentServerCountry(),
                     settingsHolder->currentServerCity());

  logger.debug() << m_countryCode << m_countryName << m_cityName;
  return true;
}

void ServerData::writeSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setCurrentServerCountryCode(m_countryCode);
  settingsHolder->setCurrentServerCountry(m_countryName);
  settingsHolder->setCurrentServerCity(m_cityName);
}

void ServerData::initialize(const ServerCountry& country,
                            const ServerCity& city) {
  logger.debug() << "Country:" << country.name() << "City:" << city.name();

  initializeInternal(country.code(), country.name(), city.name());
  emit changed();
}

void ServerData::update(const QString& countryCode, const QString& countryName,
                        const QString& cityName) {
  initializeInternal(countryCode, countryName, cityName);
  emit changed();
}

void ServerData::initializeInternal(const QString& countryCode,
                                    const QString& countryName,
                                    const QString& cityName) {
  m_initialized = true;
  m_countryCode = countryCode;
  m_countryName = countryName;
  m_cityName = cityName;
}

QString ServerData::localizedCountryName() const {
  return ServerI18N::translateCountryName(m_countryCode, m_countryName);
}

QString ServerData::localizedCityName() const {
  return ServerI18N::translateCityName(m_countryCode, m_cityName);
}
