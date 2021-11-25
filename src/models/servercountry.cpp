/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountry.h"
#include "collator.h"
#include "leakdetector.h"
#include "serverdata.h"
#include "serveri18n.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

ServerCountry::ServerCountry() { MVPN_COUNT_CTOR(ServerCountry); }

ServerCountry::ServerCountry(const ServerCountry& other) {
  MVPN_COUNT_CTOR(ServerCountry);
  *this = other;
}

ServerCountry& ServerCountry::operator=(const ServerCountry& other) {
  if (this == &other) return *this;

  m_name = other.m_name;
  m_code = other.m_code;
  m_cities = other.m_cities;

  return *this;
}

ServerCountry::~ServerCountry() { MVPN_COUNT_DTOR(ServerCountry); }

bool ServerCountry::fromJson(const QJsonObject& countryObj) {
  QJsonValue countryName = countryObj.value("name");
  if (!countryName.isString()) {
    return false;
  }

  QJsonValue countryCode = countryObj.value("code");
  if (!countryCode.isString()) {
    return false;
  }

  QJsonValue cities = countryObj.value("cities");
  if (!cities.isArray()) {
    return false;
  }

  QList<ServerCity> scList;
  QJsonArray citiesArray = cities.toArray();
  for (QJsonValue cityValue : citiesArray) {
    if (!cityValue.isObject()) {
      return false;
    }

    QJsonObject cityObject = cityValue.toObject();

    ServerCity serverCity;
    if (!serverCity.fromJson(cityObject)) {
      return false;
    }

    scList.append(serverCity);
  }

  m_name = countryName.toString();
  m_code = countryCode.toString();
  m_cities.swap(scList);

  sortCities();

  return true;
}

const QList<Server> ServerCountry::servers(const ServerData& data) const {
  for (const ServerCity& city : m_cities) {
    if (city.name() == data.exitCityName()) {
      return city.servers();
    }
  }

  return QList<Server>();
}

namespace {

bool sortCityCallback(const ServerCity& a, const ServerCity& b,
                      const QString& countryCode, Collator* collator) {
  Q_ASSERT(collator);
  return collator->compare(
             ServerI18N::translateCityName(countryCode, a.name()),
             ServerI18N::translateCityName(countryCode, b.name())) < 0;
}

}  // anonymous namespace

void ServerCountry::sortCities() {
  Collator collator;

  std::sort(m_cities.begin(), m_cities.end(),
            std::bind(sortCityCallback, std::placeholders::_1,
                      std::placeholders::_2, m_code, &collator));
}
