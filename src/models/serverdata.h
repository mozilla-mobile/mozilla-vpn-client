/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
    [[nodiscard]] bool fromSettings(SettingsHolder &settingsHolder);

    void writeSettings(SettingsHolder &settingsHolder);

    void initialize(const ServerCountry &country, const ServerCity &city);

    bool initialized() const { return m_initialized; }

    const QString &countryCode() const { return m_countryCode; }

    const QString &country() const { return m_country; }

    const QString &city() const { return m_city; }

    void forget() { m_initialized = false; }

    void update(const QString &countryCode, const QString &country, const QString &city);

signals:
    void changed();

private:
    void initializeInternal(const QString &countryCode, const QString &country, const QString &city);

private:
    bool m_initialized = false;

    QString m_countryCode;
    QString m_country;
    QString m_city;
};

#endif // SERVERDATA_H
