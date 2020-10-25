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

// static
ServerCountry ServerCountry::fromJson(QJsonObject &countryObj)
{
    Q_ASSERT(countryObj.contains("name"));
    QJsonValue countryName = countryObj.take("name");
    Q_ASSERT(countryName.isString());

    Q_ASSERT(countryObj.contains("code"));
    QJsonValue countryCode = countryObj.take("code");
    Q_ASSERT(countryCode.isString());

    Q_ASSERT(countryObj.contains("cities"));
    QJsonValue cities = countryObj.take("cities");
    Q_ASSERT(cities.isArray());

    ServerCountry sc(countryName.toString(), countryCode.toString());

    QJsonArray citiesArray = cities.toArray();
    for (QJsonValue cityValue : citiesArray) {
        Q_ASSERT(cityValue.isObject());
        QJsonObject cityObject = cityValue.toObject();

        ServerCity serverCity;
        if (!serverCity.fromJson(cityObject)) {
            // TODO
        }

        sc.m_cities.append(serverCity);
    }

    std::sort(sc.m_cities.begin(), sc.m_cities.end(), sortCityCallback);

    return sc;
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
