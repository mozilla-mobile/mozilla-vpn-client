#include "serverdata.h"
#include "servercountrymodel.h"

#include <QDebug>
#include <QSettings>

bool ServerData::fromSettings(QSettings &settings)
{
    if (!settings.contains("currentServer/name")) {
        return false;
    }

    m_serverName = settings.value("currentServer/name").toString();
    m_countryCode = settings.value("currentServer/country").toString();
    m_city = settings.value("currentServer/city").toString();

    qDebug() << m_serverName << m_countryCode << m_city;

    m_initialized = true;

    return true;
}

void ServerData::writeSettings(QSettings &settings)
{
    settings.setValue("currentServer/name", m_serverName);
    settings.setValue("currentServer/country", m_countryCode);
    settings.setValue("currentServer/city", m_city);
}

void ServerData::initialize(const ServerCountry &country,
                            const ServerCity &city,
                            const Server &server)
{
    qDebug() << "Country:" << country.name() << "City:" << city.name()
             << "Sever:" << server.hostname();

    m_initialized = true;

    m_serverName = server.hostname();
    m_countryCode = country.code();
    m_city = city.name();

    emit changed();
}
