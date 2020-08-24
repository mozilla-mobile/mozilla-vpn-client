#include "serverdata.h"
#include "servercountrymodel.h"

#include <QDebug>
#include <QSettings>

bool ServerData::fromSettings(QSettings &settings)
{
    if (!settings.contains("currentServer/country") || !settings.contains("currentServer/city")) {
        return false;
    }

    m_countryCode = settings.value("currentServer/country").toString();
    m_city = settings.value("currentServer/city").toString();

    qDebug() << m_countryCode << m_city;

    m_initialized = true;

    return true;
}

void ServerData::writeSettings(QSettings &settings)
{
    settings.setValue("currentServer/country", m_countryCode);
    settings.setValue("currentServer/city", m_city);
}

void ServerData::initialize(const ServerCountry &country, const ServerCity &city)
{
    qDebug() << "Country:" << country.name() << "City:" << city.name();

    m_initialized = true;

    m_countryCode = country.code();
    m_city = city.name();

    emit changed();
}
