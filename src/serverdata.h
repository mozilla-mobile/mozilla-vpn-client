#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QObject>

class QSettings;
class ServerCountryModel;
class ServerCountry;
class ServerCity;
class Server;

class ServerData final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString countryCode READ countryCode)
    Q_PROPERTY(QString city READ city)

public:
    bool fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    void initialize(const ServerCountry &country, const ServerCity &city, const Server &server);

    bool initialized() const { return m_initialized; }

    const QString &countryCode() const { return m_countryCode; }

    const QString &city() const { return m_city; }

private:
    bool m_initialized = false;

    QString m_serverName;
    QString m_countryCode;
    QString m_city;
};

#endif // SERVERDATA_H
