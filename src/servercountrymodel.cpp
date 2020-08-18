#include "servercountrymodel.h"

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

        //m_countries.append(Country(countryName.toString(), countryCode.toString()));
    }
}

void ServerCountryModel::writeSettings(QSettings &settings)
{
    settings.setValue("servers", m_rawJson);
}
