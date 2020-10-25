/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountry.h"
#include "serverdata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

namespace {

bool sortCityCallback(const ServerCity &a, const ServerCity &b)
{
    return a.name() < b.name();
}

} // anonymous namespace

bool ServerCountry::fromJson(QJsonObject &countryObj)
{
    QJsonValue countryName = countryObj.take("name");
    if (!countryName.isString()) {
        return false;
    }

    QJsonValue countryCode = countryObj.take("code");
    if (!countryCode.isString()) {
        return false;
    }

    QJsonValue cities = countryObj.take("cities");
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

    std::sort(m_cities.begin(), m_cities.end(), sortCityCallback);

    return true;
}

const QList<Server> ServerCountry::getServers(const ServerData &data) const
{
    for (const ServerCity &city : m_cities) {
        if (city.name() == data.city()) {
            return city.getServers();
        }
    }

    return QList<Server>();
}
