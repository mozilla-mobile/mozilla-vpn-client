/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountrymodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "appconstants.h"
#include "collator.h"
#include "feature.h"
#include "leakdetector.h"
#include "location.h"
#include "logger.h"
#include "mozillavpn.h"
#include "servercountry.h"
#include "serverdata.h"
#include "serveri18n.h"
#include "settingsholder.h"

namespace {
Logger logger("ServerCountryModel");
}

ServerCountryModel::ServerCountryModel() { MZ_COUNT_CTOR(ServerCountryModel); }

ServerCountryModel::~ServerCountryModel() { MZ_COUNT_DTOR(ServerCountryModel); }

bool ServerCountryModel::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading the server list from settings";

  const QByteArray json = settingsHolder->servers();
  if (json.isEmpty() || !fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

bool ServerCountryModel::fromJson(const QByteArray& s) {
  logger.debug() << "Reading from JSON";

  if (!s.isEmpty() && m_rawJson == s) {
    logger.debug() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(s)) {
    return false;
  }

  m_rawJson = s;
  emit changed();
  return true;
}

bool ServerCountryModel::fromJsonInternal(const QByteArray& s) {
  beginResetModel();

  m_rawJson = "";
  m_countries.clear();
  m_servers.clear();
  m_sumLatencyMsec = 0;
  m_numLatencySamples = 0;

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

      QJsonArray serverArray = servers.toArray();
      for (const QJsonValue& serverValue : serverArray) {
        Server server;
        if (!server.fromJson(serverValue.toObject())) {
          return false;
        }
        m_servers[server.publicKey()] = server;
      }
    }
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
      for (const ServerCity& city : country.cities()) {
        list.append(QVariant::fromValue(&city));
      }

      return QVariant(list);
    }

    default:
      return QVariant();
  }
}

int ServerCountryModel::cityConnectionScore(const QString& countryCode,
                                            const QString& cityCode) const {
  for (const ServerCity& city : cities(countryCode)) {
    if (city.code() == cityCode) {
      return city.connectionScore();
    }
  }

  // No such country was found.
  return NoData;
}

QStringList ServerCountryModel::pickRandom() const {
  logger.debug() << "Choosing a random server";
  qsizetype index = QRandomGenerator::global()->generate() % m_servers.count();

  // Iterate to find the selected country and city. This winds up weighting the
  // choice of city proportional to the number of servers hosted there.
  for (auto country = m_countries.cbegin(); country != m_countries.cend();
       country++) {
    for (auto city = country->cities().cbegin();
         city != country->cities().cend(); city++) {
      if (index >= city->servers().count()) {
        // Keep searching.
        index -= city->servers().count();
      } else {
        // We found our selection.
        QStringList serverChoice = {
            city->country(), city->name(),
            ServerI18N::translateCityName(city->country(), city->name())};
        return serverChoice;
      }
    }
  }

  // We should not get here, unless the model has more entries in m_servers()
  // than actually exist in the country and city lists.
  Q_ASSERT(false);
  return QStringList();
}

// Select the city that we think is going to perform the best
QStringList ServerCountryModel::pickBest() const {
  QList<QVariant> list = recommendedLocations(1);
  if (list.isEmpty()) {
    return QStringList();
  }

  QVariant qv = list.first();
  Q_ASSERT(qv.canConvert<const ServerCity*>());
  const ServerCity* city = qv.value<const ServerCity*>();
  return QStringList({city->country(), city->name()});
}

bool ServerCountryModel::exists(const QString& countryCode,
                                const QString& cityName) const {
  logger.debug() << "Check if the server is still valid.";

  for (const ServerCity& city : cities(countryCode)) {
    if (cityName == city.name()) {
      return true;
    }
  }

  return false;
}

const QList<ServerCity>& ServerCountryModel::cities(
    const QString& countryCode) const {
  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      return country.cities();
    }
  }

  static const QList<ServerCity> emptylist;
  return emptylist;
}

const QList<Server> ServerCountryModel::servers(const QString& countryCode,
                                                const QString& cityName) const {
  QList<Server> results;
  for (const ServerCity& city : cities(countryCode)) {
    if (city.name() != cityName) {
      continue;
    }

    for (const QString& pubkey : city.servers()) {
      if (m_servers.contains(pubkey)) {
        results.append(m_servers.value(pubkey));
      }
    }
    break;
  }

  return results;
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

unsigned int ServerCountryModel::avgLatency() const {
  if (m_numLatencySamples == 0) {
    return 0;
  }
  return (m_sumLatencyMsec + m_numLatencySamples - 1) / m_numLatencySamples;
}

void ServerCountryModel::setServerLatency(const QString& publicKey,
                                          unsigned int msec) {
  if (!m_servers.contains(publicKey)) {
    return;
  }

  Server& server = m_servers[publicKey];
  if (server.latency() != 0) {
    m_sumLatencyMsec -= server.latency();
  } else {
    m_numLatencySamples++;
  }
  m_sumLatencyMsec += msec;
  server.setLatency(msec);

  // Ugly iteration. Find the city for this server.
  // TODO: Maybe it would be cleaner to move the latency data into ServerCity.
  for (const ServerCountry& country : m_countries) {
    for (const ServerCity& city : country.cities()) {
      if (city.servers().contains(publicKey)) {
        emit city.scoreChanged();
        return;
      }
    }
  }
}

void ServerCountryModel::clearServerLatency() {
  // Invalidate the latency data.
  m_sumLatencyMsec = 0;
  m_numLatencySamples = 0;
  for (Server& server : m_servers) {
    server.setLatency(0);
  }

  // Emit changed signals for the connection scores.
  for (const ServerCountry& country : m_countries) {
    for (const ServerCity& city : country.cities()) {
      emit city.scoreChanged();
    }
  }
}

void ServerCountryModel::setServerCooldown(const QString& publicKey) {
  if (m_servers.contains(publicKey)) {
    m_servers[publicKey].setCooldownTimeout(
        AppConstants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);
  }
}

void ServerCountryModel::setCooldownForAllServersInACity(
    const QString& countryCode, const QString& cityCode) {
  logger.debug() << "Set cooldown for all servers for: "
                 << logger.sensitive(countryCode) << logger.sensitive(cityCode);

  for (const ServerCity& city : cities(countryCode)) {
    if (city.code() != cityCode) {
      continue;
    }
    for (const QString& pubkey : city.servers()) {
      setServerCooldown(pubkey);
    }
  }
}

QList<QVariant> ServerCountryModel::recommendedLocations(
    unsigned int maxResults) const {
  double latencyScale = avgLatency();
  if (latencyScale < 100.0) {
    latencyScale = 100.0;
  }

  QVector<QVariant> cityResults;
  QVector<double> rankResults;
  cityResults.reserve(maxResults + 1);
  rankResults.reserve(maxResults + 1);
  for (const ServerCountry& country : m_countries) {
    for (const ServerCity& city : country.cities()) {
      double cityRanking = city.connectionScore() * 256.0;

      // For tiebreaking, use the geographic distance and latency.
      double distance = MozillaVPN::instance()->location()->distance(
          city.latitude(), city.longitude());
      cityRanking -= city.latency() / latencyScale;
      cityRanking -= distance;

#ifdef MZ_DEBUG
      logger.debug() << "Evaluating" << city.name() << "-" << city.latency()
                     << "ms"
                     << "-" << QString::number(distance) << "-"
                     << QString::number(cityRanking);
#endif

      // Insert into the result list
      int i;
      for (i = 0; i < rankResults.count(); i++) {
        if (rankResults[i] < cityRanking) {
          break;
        }
      }
      if (i < maxResults) {
        rankResults.insert(i, cityRanking);
        cityResults.insert(i, QVariant::fromValue(&city));
      }
      if (rankResults.count() > maxResults) {
        rankResults.resize(maxResults);
        cityResults.resize(maxResults);
      }
    }
  }

  return cityResults.toList();
}

namespace {

bool sortCountryCallback(const ServerCountry& a, const ServerCountry& b,
                         Collator* collator) {
  Q_ASSERT(collator);
  return collator->compare(
             ServerI18N::translateCountryName(a.code(), a.name()),
             ServerI18N::translateCountryName(b.code(), b.name())) < 0;
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
