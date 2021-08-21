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

  if (!settingsHolder->hasCurrentServerCountryCode() ||
      !settingsHolder->hasCurrentServerCity()) {
    return false;
  }

  initializeInternal(settingsHolder->currentServerCountryCode(),
                     settingsHolder->currentServerCity());

  if (settingsHolder->hasEntryServerCountryCode() &&
      settingsHolder->hasEntryServerCity()) {
    initializeEntryServer(settingsHolder->entryServerCountryCode(),
                          settingsHolder->entryServerCity());
  }

  logger.debug() << toString();
  return true;
}

bool ServerData::fromString(const QString& data) {
  QStringList serverList = data.split("->");

  QStringList current = serverList.last().split(",");
  if (current.count() != 2) {
    return false;
  }
  initializeInternal(current[1].trimmed(), current[0].trimmed());

  if (serverList.count() > 1) {
    QStringList entry = serverList.first().split(",");
    if (entry.count() == 2) {
      initializeEntryServer(entry[1].trimmed(), entry[0].trimmed());
    }
  }

  logger.debug() << toString();
  return true;
}

void ServerData::writeSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setCurrentServerCountryCode(m_countryCode);
  settingsHolder->setCurrentServerCity(m_cityName);

  if (m_multihop) {
    settingsHolder->setEntryServerCountryCode(m_entryCountryCode);
    settingsHolder->setEntryServerCity(m_entryCityName);
  } else {
    settingsHolder->removeEntryServer();
  }
}

void ServerData::update(const QString& countryCode, const QString& cityName,
                        const QString& entryCountryCode,
                        const QString& entryCityName) {
  logger.debug() << "Country:" << countryCode << "City:" << cityName;
  initializeInternal(countryCode, cityName);
  if (!entryCountryCode.isNull() && !entryCityName.isNull()) {
    initializeEntryServer(entryCountryCode, entryCityName);
  }

  emit changed();
}

void ServerData::initializeInternal(const QString& countryCode,
                                    const QString& cityName) {
  m_initialized = true;
  m_countryCode = countryCode;
  m_cityName = cityName;
}

void ServerData::initializeEntryServer(const QString& countryCode,
                                       const QString& cityName) {
  m_multihop = true;
  m_entryCountryCode = countryCode;
  m_entryCityName = cityName;
}

QString ServerData::localizedCityName() const {
  return ServerI18N::translateCityName(m_countryCode, m_cityName);
}

QString ServerData::localizedEntryCity() const {
  return ServerI18N::translateCityName(m_entryCountryCode, m_entryCityName);
}

QString ServerData::toString() const {
  QString result = "";
  if (m_multihop) {
    result += m_entryCityName + ", " + m_entryCountryCode + " -> ";
  }

  result += m_cityName + ", " + m_countryCode;
  return result;
}
