/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QObject>
#include <QSettings>

class SettingsHolder : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool ipv6 READ ipv6 WRITE setIpv6 NOTIFY ipv6Changed)
    Q_PROPERTY(bool localNetwork READ localNetwork WRITE setLocalNetwork NOTIFY localNetworkChanged)

public:
    SettingsHolder();

    void clear();

#define GETSET(type, has, get, set) \
    bool has() const; \
    type get() const; \
    void set(const type &value);

    GETSET(bool, hasIpv6, ipv6, setIpv6)
    GETSET(bool, hasLocalNetwork, localNetwork, setLocalNetwork)
    GETSET(QString, hasLanguage, language, setLanguage)
    GETSET(QString, hasToken, token, setToken)
    GETSET(QString, hasPrivateKey, privateKey, setPrivateKey)
    GETSET(QByteArray, hasServers, servers, setServers)
    GETSET(QString, hasUserAvatar, userAvatar, setUserAvatar)
    GETSET(QString, hasUserDisplayName, userDisplayName, setUserDisplayName)
    GETSET(QString, hasUserEmail, userEmail, setUserEmail)
    GETSET(int, hasUserMaxDevices, userMaxDevices, setUserMaxDevices)
    GETSET(bool, hasUserSubscriptionNeeded, userSubscriptionNeeded, setUserSubscriptionNeeded)
    GETSET(QString, hasCurrentServerCountry, currentServerCountry, setCurrentServerCountry)
    GETSET(QString, hasCurrentServerCity, currentServerCity, setCurrentServerCity)
    GETSET(QByteArray, hasDevices, devices, setDevices)
#undef GETSET

signals:
    void ipv6Changed();
    void localNetworkChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGSHOLDER_H
