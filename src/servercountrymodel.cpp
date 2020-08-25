#include "servercountrymodel.h"
#include "servercountry.h"
#include "serverdata.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSettings>

bool ServerCountryModel::fromSettings(QSettings &settings)
{
    qDebug() << "Reading the server list from settings";

    if (!settings.contains("servers")) {
        return false;
    }

    m_rawJson = settings.value("servers").toByteArray();
    fromJsonInternal();

    return true;
}

void ServerCountryModel::fromJson(const QByteArray &s)
{
    qDebug() << "Reading from JSON";

    if (m_rawJson == s) {
        qDebug() << "Nothing has changed";
        return;
    }

    m_rawJson = s;
    fromJsonInternal();
}

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
    for (QJsonArray::Iterator i = countriesArray.begin(); i != countriesArray.end(); ++i) {
        Q_ASSERT(i->isObject());
        QJsonObject countryObj = i->toObject();
        m_countries.append(ServerCountry::fromJson(countryObj));
    }

    endResetModel();
}

void ServerCountryModel::writeSettings(QSettings &settings)
{
    settings.setValue("servers", m_rawJson);
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

        for (QList<ServerCity>::ConstIterator i = cities.begin(); i != cities.end(); ++i) {
            list.append(i->name());
        }

        return QVariant(list);
    }

    default:
        return QVariant();
    }
}

void ServerCountryModel::pickRandom(ServerData &data)
{
    qDebug() << "Choosing a random server";

    quint32 countryId = QRandomGenerator::global()->generate() % m_countries.length();
    const ServerCountry &country = m_countries[countryId];

    quint32 cityId = QRandomGenerator::global()->generate() % country.cities().length();
    const ServerCity &city = country.cities().at(cityId);

    data.initialize(country, city);
}

const QList<Server> ServerCountryModel::getServers(const ServerData &data) const
{
    for (QList<ServerCountry>::ConstIterator i = m_countries.begin(); i != m_countries.end(); ++i) {
        if (i->code() == data.countryCode()) {
            return i->getServers(data);
        }
    }

    return QList<Server>();
}
