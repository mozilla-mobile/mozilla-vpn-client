#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QObject>

class ServerCountryModel;
class ServerCountry;
class ServerCity;
class Server;
class SettingsHolder;

class ServerData final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString countryCode READ countryCode NOTIFY changed)
    Q_PROPERTY(QString city READ city NOTIFY changed)

public:
    bool fromSettings(SettingsHolder &settingsHolder);

    void writeSettings(SettingsHolder &settingsHolder);

    void initialize(const ServerCountry &country, const ServerCity &city);

    bool initialized() const { return m_initialized; }

    const QString &countryCode() const { return m_countryCode; }

    const QString &city() const { return m_city; }

    void forget() { m_initialized = false; }

    void update(const QString &countryCode, const QString &city);

signals:
    void changed();

private:
    bool m_initialized = false;

    QString m_countryCode;
    QString m_city;
};

#endif // SERVERDATA_H
