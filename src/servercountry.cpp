#include "servercountry.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

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
    for (QJsonArray::Iterator i = citiesArray.begin(); i != citiesArray.end(); ++i) {
        Q_ASSERT(i->isObject());
        QJsonObject city = i->toObject();
        sc.m_cities.append(ServerCity::fromJson(city));
    }

    return sc;
}

const QStringList ServerCountry::cities() const
{
    QStringList list;

    for (QList<ServerCity>::ConstIterator i = m_cities.begin(); i != m_cities.end(); ++i) {
        list.append(i->name());
    }

    return list;
}
