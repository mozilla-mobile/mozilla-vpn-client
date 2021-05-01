/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountrymodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "servercountry.h"
#include "serverdata.h"
#include "serveri18n.h"
#include "settingsholder.h"

#include <QCollator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

namespace {
Logger logger(LOG_MODEL, "ServerCountryModel");
}

ServerCountryModel::ServerCountryModel() {
  MVPN_COUNT_CTOR(ServerCountryModel);
}

ServerCountryModel::~ServerCountryModel() {
  MVPN_COUNT_DTOR(ServerCountryModel);
}

bool ServerCountryModel::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.log() << "Reading the server list from settings";

  if (!settingsHolder->hasServers()) {
    return false;
  }

  const QByteArray json = settingsHolder->servers();
  if (!fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

bool ServerCountryModel::fromJson(const QByteArray& s) {
  logger.log() << "Reading from JSON";

  if (!s.isEmpty() && m_rawJson == s) {
    logger.log() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(s)) {
    return false;
  }

  m_rawJson = s;
  return true;
}

bool ServerCountryModel::fromJsonInternal(const QByteArray& s) {
  beginResetModel();

  m_rawJson = "";
  m_countries.clear();

  QJsonDocument doc = QJsonDocument::fromJson(s);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  QJsonValue countries = obj.value("countries");
  if (!countries.isArray()) {
    return false;
  }

  QJsonArray countriesArray = countries.toArray();
  for (QJsonValue countryValue : countriesArray) {
    if (!countryValue.isObject()) {
      return false;
    }

    QJsonObject countryObj = countryValue.toObject();

    ServerCountry country;
    if (!country.fromJson(countryObj)) {
      return false;
    }

    m_countries.append(country);
  }

  sortCountries();

  endResetModel();

  return true;
}

QHash<int, QByteArray> ServerCountryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[LocalizedNameRole] = "localizedName";
  roles[CodeRole] = "code";
  roles[CitiesRole] = "cities";
  return roles;
}

QVariant ServerCountryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_countries.length()) {
    return QVariant();
  }

  switch (role) {
    case NameRole:
      return QVariant(m_countries.at(index.row()).name());

    case LocalizedNameRole: {
      const ServerCountry& country = m_countries.at(index.row());
      return QVariant(
          ServerI18N::translateCountryName(country.code(), country.name()));
    }

    case CodeRole:
      return QVariant(m_countries.at(index.row()).code());

    case CitiesRole: {
      const ServerCountry& country = m_countries.at(index.row());

      QList<QVariant> list;
      const QList<ServerCity>& cities = country.cities();
      for (const ServerCity& city : cities) {
        QStringList names{city.name(), ServerI18N::translateCityName(
                                           country.code(), city.name())};
        list.append(QVariant(names));
      }

      return QVariant(list);
    }

    default:
      return QVariant();
  }
}

bool ServerCountryModel::pickIfExists(const QString& countryCode,
                                      const QString& cityCode,
                                      ServerData& data) const {
  logger.log() << "Checking if the server exists" << countryCode << cityCode;

  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      for (const ServerCity& city : country.cities()) {
        if (city.code() == cityCode) {
          data.initialize(country, city);
          return true;
        }
      }
      break;
    }
  }

  return false;
}

void ServerCountryModel::pickRandom(ServerData& data) const {
  logger.log() << "Choosing a random server";

  quint32 countryId =
      QRandomGenerator::global()->generate() % m_countries.length();
  const ServerCountry& country = m_countries[countryId];

  quint32 cityId =
      QRandomGenerator::global()->generate() % country.cities().length();
  const ServerCity& city = country.cities().at(cityId);

  data.initialize(country, city);
}

bool ServerCountryModel::pickByIPv4Address(const QString& ipv4Address,
                                           ServerData& data) const {
  logger.log() << "Choosing a server with addres:" << ipv4Address;

  for (const ServerCountry& country : m_countries) {
    for (const ServerCity& city : country.cities()) {
      for (const Server& server : city.servers()) {
        if (server.ipv4AddrIn() == ipv4Address) {
          data.initialize(country, city);
          return true;
        }
      }
    }
  }

  return false;
}

bool ServerCountryModel::exists(ServerData& data) const {
  logger.log() << "Check if the server is still valid.";
  Q_ASSERT(data.initialized());

  for (const ServerCountry& country : m_countries) {
    if (country.code() == data.countryCode()) {
      for (const ServerCity& city : country.cities()) {
        if (data.cityName() == city.name()) {
          return true;
        }
      }

      break;
    }
  }

  return false;
}

const QList<Server> ServerCountryModel::servers(const ServerData& data) const {
  for (const ServerCountry& country : m_countries) {
    if (country.code() == data.countryCode()) {
      return country.servers(data);
    }
  }

  return QList<Server>();
}

const QString ServerCountryModel::countryName(
    const QString& countryCode) const {
  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      return country.name();
    }
  }

  return QString();
}

void ServerCountryModel::retranslate() {
  beginResetModel();
  sortCountries();
  endResetModel();
}

namespace {

bool sortCountryCallback(const ServerCountry& a, const ServerCountry& b,
                         QCollator* collator) {
  Q_ASSERT(collator);
  return collator->compare(
             ServerI18N::translateCountryName(a.code(), a.name()),
             ServerI18N::translateCountryName(b.code(), b.name())) < 0;
}

}  // anonymous namespace

void ServerCountryModel::sortCountries() {
  QCollator collator;
  std::sort(m_countries.begin(), m_countries.end(),
            std::bind(sortCountryCallback, std::placeholders::_1,
                      std::placeholders::_2, &collator));

  for (ServerCountry& country : m_countries) {
    country.sortCities();
  }
}
