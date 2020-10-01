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

    Q_PROPERTY(bool ipv6Enabled READ ipv6Enabled WRITE setIpv6Enabled NOTIFY ipv6EnabledChanged)
    Q_PROPERTY(bool localNetworkAccess READ localNetworkAccess WRITE setLocalNetworkAccess NOTIFY
                   localNetworkAccessChanged)
    Q_PROPERTY(bool unsecuredNetworkAlert READ unsecuredNetworkAlert WRITE setUnsecuredNetworkAlert
                   NOTIFY unsecuredNetworkAlertChanged)
    Q_PROPERTY(bool captivePortalAlert READ captivePortalAlert WRITE setCaptivePortalAlert NOTIFY
                   captivePortalAlertChanged)

public:
    SettingsHolder();

    void clear();

#define GETSET(type, has, get, set) \
    bool has() const; \
    type get() const; \
    void set(const type &value);

    GETSET(bool, hasIpv6Enabled, ipv6Enabled, setIpv6Enabled)
    GETSET(bool, hasLocalNetworkAccess, localNetworkAccess, setLocalNetworkAccess)
    GETSET(bool, hasUnsecuredNetworkAlert, unsecuredNetworkAlert, setUnsecuredNetworkAlert)
    GETSET(bool, hasCaptivePortalAlert, captivePortalAlert, setCaptivePortalAlert)
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
    void ipv6EnabledChanged();
    void localNetworkAccessChanged();
    void unsecuredNetworkAlertChanged();
    void captivePortalAlertChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGSHOLDER_H
