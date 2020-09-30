#include "settingsholder.h"

#include <QDebug>
#include <QSettings>

constexpr bool SETTINGS_IPV6_DEFAULT = true;
constexpr bool SETTINGS_LOCALNETWORK_DEFAULT = false;

constexpr const char *SETTINGS_IPV6 = "ipv6";
constexpr const char *SETTINGS_LOCALNETWORK = "localNetwork";
constexpr const char *SETTINGS_LANGUAGE = "language";
constexpr const char *SETTINGS_TOKEN = "token";
constexpr const char *SETTINGS_SERVERS = "servers";
constexpr const char *SETTINGS_PRIVATEKEY = "privateKey";
constexpr const char *SETTINGS_USER_AVATAR = "user/avatar";
constexpr const char *SETTINGS_USER_DISPLAYNAME = "user/displayName";
constexpr const char *SETTINGS_USER_EMAIL = "user/email";
constexpr const char *SETTINGS_USER_MAXDEVICES = "user/maxDevices";
constexpr const char *SETTINGS_USER_SUBSCRIPTIONNEEDED = "user/subscriptionNeeded";
constexpr const char *SETTINGS_CURRENTSERVER_COUNTRY = "currentServer/country";
constexpr const char *SETTINGS_CURRENTSERVER_CITY = "currentServer/city";
constexpr const char *SETTINGS_DEVICES = "devices";

SettingsHolder::SettingsHolder() : m_settings("mozilla", "vpn") {}

void SettingsHolder::clear()
{
    qDebug() << "Clean up the settings";

    m_settings.remove(SETTINGS_TOKEN);
    m_settings.remove(SETTINGS_SERVERS);
    m_settings.remove(SETTINGS_PRIVATEKEY);
    m_settings.remove(SETTINGS_USER_AVATAR);
    m_settings.remove(SETTINGS_USER_DISPLAYNAME);
    m_settings.remove(SETTINGS_USER_EMAIL);
    m_settings.remove(SETTINGS_USER_MAXDEVICES);
    m_settings.remove(SETTINGS_USER_SUBSCRIPTIONNEEDED);
    m_settings.remove(SETTINGS_CURRENTSERVER_COUNTRY);
    m_settings.remove(SETTINGS_CURRENTSERVER_CITY);
    m_settings.remove(SETTINGS_DEVICES);

    // We do not remove language, ipv6 and localnetwork settings.
}

bool SettingsHolder::ipv6() const
{
    if (!m_settings.contains(SETTINGS_IPV6)) {
        return SETTINGS_IPV6_DEFAULT;
    }

    return m_settings.value(SETTINGS_IPV6).toBool();
}

void SettingsHolder::setIpv6(bool ipv6)
{
    qDebug() << "Setting Ipv6:" << ipv6;
    m_settings.setValue(SETTINGS_IPV6, ipv6);
}

bool SettingsHolder::localNetwork() const
{
    if (!m_settings.contains(SETTINGS_LOCALNETWORK)) {
        return SETTINGS_LOCALNETWORK_DEFAULT;
    }

    return m_settings.value(SETTINGS_LOCALNETWORK).toBool();
}

void SettingsHolder::setLocalNetwork(bool localNetwork)
{
    qDebug() << "Setting LocalNetwork:" << localNetwork;
    m_settings.setValue(SETTINGS_LOCALNETWORK, localNetwork);
}

QString SettingsHolder::language() const
{
    if (m_settings.contains(SETTINGS_LANGUAGE)) {
        return m_settings.value(SETTINGS_LANGUAGE).toString();
    }
    return QString();
}

void SettingsHolder::setLanguage(const QString &language)
{
    qDebug() << "Setting language:" << language;
    m_settings.setValue(SETTINGS_LANGUAGE, language);
}

#define GETSET(type, toType, key, has, get, set) \
    bool SettingsHolder::has() const { return m_settings.contains(key); } \
    type SettingsHolder::get() const \
    { \
        Q_ASSERT(has()); \
        return m_settings.value(key).toType(); \
    } \
    void SettingsHolder::set(const type &value) { m_settings.setValue(key, value); }

GETSET(QString, toString, SETTINGS_TOKEN, hasToken, token, setToken)
GETSET(QString, toString, SETTINGS_PRIVATEKEY, hasPrivateKey, privateKey, setPrivateKey)
GETSET(QByteArray, toByteArray, SETTINGS_SERVERS, hasServers, servers, setServers)
GETSET(QString, toString, SETTINGS_USER_AVATAR, hasUserAvatar, userAvatar, setUserAvatar)
GETSET(QString,
       toString,
       SETTINGS_USER_DISPLAYNAME,
       hasUserDisplayName,
       userDisplayName,
       setUserDisplayName)
GETSET(QString, toString, SETTINGS_USER_EMAIL, hasUserEmail, userEmail, setUserEmail)
GETSET(int, toInt, SETTINGS_USER_MAXDEVICES, hasUserMaxDevices, userMaxDevices, setUserMaxDevices)
GETSET(bool,
       toBool,
       SETTINGS_USER_SUBSCRIPTIONNEEDED,
       hasUserSubscriptionNeeded,
       userSubscriptionNeeded,
       setUserSubscriptionNeeded)
GETSET(QString,
       toString,
       SETTINGS_CURRENTSERVER_COUNTRY,
       hasCurrentServerCountry,
       currentServerCountry,
       setCurrentServerCountry)
GETSET(QString,
       toString,
       SETTINGS_CURRENTSERVER_CITY,
       hasCurrentServerCity,
       currentServerCity,
       setCurrentServerCity)
GETSET(QByteArray, toByteArray, SETTINGS_DEVICES, hasDevices, devices, setDevices)

#undef GETSET
