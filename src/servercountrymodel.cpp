/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountrymodel.h"
#include "logger.h"
#include "servercountry.h"
#include "serverdata.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

namespace {
Logger logger(LOG_MODEL, "ServerCountryModel");
}

bool ServerCountryModel::fromSettings(SettingsHolder &settingsHolder)
{
    logger.log() << "Reading the server list from settings";

    if (!settingsHolder.hasServers()) {
        return false;
    }

    m_rawJson = settingsHolder.servers();
    fromJsonInternal();

    return true;
}

void ServerCountryModel::fromJson(const QByteArray &s)
{
    logger.log() << "Reading from JSON";

    if (m_rawJson == s) {
        logger.log() << "Nothing has changed";
        return;
    }

    m_rawJson = s;
    fromJsonInternal();
}

namespace {

bool sortCountryCallback(const ServerCountry &a, const ServerCountry &b)
{
    return a.name() < b.name();
}

} // anonymous namespace

void ServerCountryModel::fromJsonInternal()
{
    beginResetModel();

    m_countries.clear();

    QJsonDocument doc = QJsonDocument::fromJson(m_rawJson);

    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();

    Q_ASSERT(obj.contains("countries"));
    QJsonValue countries = obj.take("countries");
    Q_ASSERT(countries.isArray());

    QJsonArray countriesArray = countries.toArray();
    for (QJsonValue countryValue : countriesArray) {
        Q_ASSERT(countryValue.isObject());
        QJsonObject countryObj = countryValue.toObject();
        m_countries.append(ServerCountry::fromJson(countryObj));
    }

    std::sort(m_countries.begin(), m_countries.end(), sortCountryCallback);

    endResetModel();
}

QHash<int, QByteArray> ServerCountryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CodeRole] = "code";
    roles[CitiesRole] = "cities";
    return roles;
}

QVariant ServerCountryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_countries.length()) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return QVariant(m_countries.at(index.row()).name());

    case CodeRole:
        return QVariant(m_countries.at(index.row()).code());

    case CitiesRole: {
        QStringList list;
        const QList<ServerCity> &cities = m_countries.at(index.row()).cities();

        for (const ServerCity &city : cities) {
            list.append(city.name());
        }

        return QVariant(list);
    }

    default:
        return QVariant();
    }
}

bool ServerCountryModel::pickIfExists(const QString &countryCode,
                                      const QString &cityCode,
                                      ServerData &data) const
{
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

void ServerCountryModel::pickRandom(ServerData &data) const
{
    logger.log() << "Choosing a random server";

    quint32 countryId = QRandomGenerator::global()->generate() % m_countries.length();
    const ServerCountry &country = m_countries[countryId];

    quint32 cityId = QRandomGenerator::global()->generate() % country.cities().length();
    const ServerCity &city = country.cities().at(cityId);

    data.initialize(country, city);
}

bool ServerCountryModel::exists(ServerData &data) const
{
    logger.log() << "Check if the server is still valid.";
    Q_ASSERT(data.initialized());

    for (const ServerCountry& country : m_countries) {
        if (country.code() == data.countryCode()) {
            for (const ServerCity& city : country.cities()) {
                if (data.city() == city.name()) {
                    return true;
                }
            }

            break;
        }
    }

    return false;
}

const QList<Server> ServerCountryModel::getServers(const ServerData &data) const
{
    for (const ServerCountry &country : m_countries) {
        if (country.code() == data.countryCode()) {
            return country.getServers(data);
        }
    }

    return QList<Server>();
}
