/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountrymodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "collator.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/servercountry.h"

namespace {
Logger logger("ServerCountryModel");
}

ServerCountryModel::ServerCountryModel() { MZ_COUNT_CTOR(ServerCountryModel); }

ServerCountryModel::~ServerCountryModel() { MZ_COUNT_DTOR(ServerCountryModel); }

bool ServerCountryModel::fromJson(const QByteArray& json) {
  logger.debug() << "Reading from JSON";

  if (!json.isEmpty() && m_rawJson == json) {
    logger.debug() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(json, false)) {
    return false;
  }

  m_rawJson = json;
  emit changed();
  return true;
}

bool ServerCountryModel::appendFromJson(const QByteArray& json) {
  logger.debug() << "Appending from JSON";

  if (json.isEmpty()) {
    logger.debug() << "Nothing to append";
    return true;
  }

  if (!fromJsonInternal(json, true)) {
    return false;
  }
  emit changed();
  return true;
}

bool ServerCountryModel::fromJsonInternal(const QByteArray& s, bool append) {
  beginResetModel();

  m_rawJson = "";
  if (!append) {
    m_countries.clear();
    m_cities.clear();
    m_servers.clear();
  }
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
  for (const QJsonValue& countryValue : countriesArray) {
    if (!countryValue.isObject()) {
      return false;
    }

    QJsonObject countryObj = countryValue.toObject();

    ServerCountry country;
    if (!country.fromJson(countryObj)) {
      return false;
    }

    if (country.cities().isEmpty()) {
      continue;
    }

    m_countries.append(country);

    QJsonValue cities = countryObj.value("cities");
    if (!cities.isArray()) {
      return false;
    }

    QJsonArray cityArray = cities.toArray();
    for (const QJsonValue& cityValue : cityArray) {
      if (!cityValue.isObject()) {
        return false;
      }
      QJsonObject cityObj = cityValue.toObject();
      QJsonValue servers = cityObj.value("servers");
      if (!servers.isArray()) {
        return false;
      }

      ServerCity city;
      if (!city.fromJson(cityObj, country.code())) {
        return false;
      }
      m_cities[city.hashKey()] = city;

      QJsonArray serverArray = servers.toArray();
      for (const QJsonValue& serverValue : serverArray) {
        Server server(country.code(), city.name());
        if (!server.fromJson(serverValue.toObject())) {
          return false;
        }
        m_servers[server.identifier()] = server;
      }
    }
  }

  // Re-apply any MASQUE servers on top of the freshly loaded WireGuard list, so
  // they survive this reset. Failure here is not fatal for the WireGuard list.
  if (!m_masqueRawJson.isEmpty()) {
    mergeMasqueChangeset(m_masqueRawJson);
  }

  sortCountries();

  endResetModel();

  return true;
}

bool ServerCountryModel::appendMasqueServers(const QByteArray& changeset) {
  logger.debug() << "Appending MASQUE servers from changeset";

  if (changeset.isEmpty()) {
    logger.debug() << "Nothing to append";
    return true;
  }

  m_masqueRawJson = changeset;

  beginResetModel();
  bool ok = mergeMasqueChangeset(changeset);
  sortCountries();
  endResetModel();

  if (ok) {
    emit changed();
  }
  return ok;
}

bool ServerCountryModel::mergeMasqueChangeset(const QByteArray& changeset) {
  QJsonDocument doc = QJsonDocument::fromJson(changeset);
  if (!doc.isObject()) {
    return false;
  }

  QJsonValue changes = doc.object().value("changes");
  if (!changes.isArray()) {
    return false;
  }

  for (const QJsonValue& countryValue : changes.toArray()) {
    if (!countryValue.isObject()) {
      continue;
    }
    QJsonObject countryObj = countryValue.toObject();

    QString name = countryObj.value("name").toString();
    if (countryObj.value("code").toString().isEmpty() || name.isEmpty()) {
      continue;
    }

    QJsonValue cities = countryObj.value("cities");
    if (!cities.isArray()) {
      continue;
    }

    // The Remote Settings changeset uses upper-case ISO codes (e.g. "JP") while
    // the WireGuard list uses lower-case ones (e.g. "jp"). Match case
    // insensitively so MASQUE locations merge into the existing country instead
    // of creating a duplicate, and store everything under the canonical code.
    const QString changesetCode = countryObj.value("code").toString();

    ServerCountry* country = nullptr;
    for (ServerCountry& c : m_countries) {
      if (c.code().compare(changesetCode, Qt::CaseInsensitive) == 0) {
        country = &c;
        break;
      }
    }
    ServerCountry newCountry;
    const bool isNewCountry = (country == nullptr);
    if (isNewCountry) {
      newCountry.initialize(changesetCode.toLower(), name);
      country = &newCountry;
    }
    const QString code = country->code();

    for (const QJsonValue& cityValue : cities.toArray()) {
      if (!cityValue.isObject()) {
        continue;
      }
      QJsonObject cityObj = cityValue.toObject();

      ServerCity city;
      if (!city.fromMasqueJson(cityObj, code)) {
        continue;
      }
      m_cities[city.hashKey()] = city;
      country->addCity(city.name());

      QJsonArray serverArray = cityObj.value("servers").toArray();
      for (const QJsonValue& serverValue : serverArray) {
        Server server(code, city.name());
        if (!server.fromMasqueJson(serverValue.toObject())) {
          continue;
        }
        m_servers[server.identifier()] = server;
      }
    }

    if (isNewCountry) {
      m_countries.append(newCountry);
    }
  }

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
      return QVariant(country.localizedName());
    }

    case CodeRole:
      return QVariant(m_countries.at(index.row()).code());

    case CitiesRole: {
      const ServerCountry& country = m_countries.at(index.row());

      QList<QVariant> list;
      for (const QString& name : country.cities()) {
        const ServerCity& city = findCity(country.code(), name);
        if (city.initialized()) {
          list.append(QVariant::fromValue(&city));
        }
      }

      return QVariant(list);
    }

    default:
      return QVariant();
  }
}

bool ServerCountryModel::exists(const QString& countryCode,
                                const QString& cityName) const {
  logger.debug() << "Check if the server is still valid.";
  return m_cities.contains(ServerCity::hashKey(countryCode, cityName));
}

ServerCity& ServerCountryModel::findCity(const QString& countryCode,
                                         const QString& cityName) {
  auto index = m_cities.find(ServerCity::hashKey(countryCode, cityName));
  if (index == m_cities.end()) {
    static ServerCity emptycity;
    return emptycity;
  }

  return *index;
}

const ServerCity& ServerCountryModel::findCity(const QString& countryCode,
                                               const QString& cityName) const {
  auto index = m_cities.constFind(ServerCity::hashKey(countryCode, cityName));
  if (index == m_cities.end()) {
    static const ServerCity emptycity;
    return emptycity;
  }

  return *index;
}

const Server& ServerCountryModel::server(const QString& pubkey) const {
  auto iterator = m_servers.constFind(pubkey);
  if (iterator != m_servers.constEnd()) {
    return iterator.value();
  }

  static const Server emptyserver;
  return emptyserver;
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
                         Collator* collator) {
  Q_ASSERT(collator);
  return collator->compare(a.localizedName(), b.localizedName()) < 0;
}

}  // anonymous namespace

void ServerCountryModel::sortCountries() {
  Collator collator;
  std::sort(m_countries.begin(), m_countries.end(),
            std::bind(sortCountryCallback, std::placeholders::_1,
                      std::placeholders::_2, &collator));

  for (ServerCountry& country : m_countries) {
    country.sortCities();
  }
}
