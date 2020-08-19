#include "servercountrymodel.h"
#include "servercountry.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

    m_rawJson = s;
    fromJsonInternal();
}

void ServerCountryModel::fromJsonInternal()
{
    for (QList<QPointer<ServerCountry>>::Iterator i = m_countries.begin(); i != m_countries.end();
         ++i) {
        delete *i;
    }

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

        Q_ASSERT(countryObj.contains("name"));
        QJsonValue countryName = countryObj.take("name");
        Q_ASSERT(countryName.isString());

        Q_ASSERT(countryObj.contains("code"));
        QJsonValue countryCode = countryObj.take("code");
        Q_ASSERT(countryCode.isString());

        m_countries.append(new ServerCountry(countryName.toString(), countryCode.toString()));
    }
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
    return roles;
}

QVariant ServerCountryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_countries.length()) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return QVariant(m_countries.at(index.row())->name());

    case CodeRole:
        return QVariant(m_countries.at(index.row())->code());

    default:
        return QVariant();
    }
}
