#include "settingsholder.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QSettings>

constexpr bool SETTINGS_IPV6_DEFAULT = true;
constexpr bool SETTINGS_LOCALNETWORK_DEFAULT = false;

constexpr const char *SETTINGS_IPV6 = "ipv6";
constexpr const char *SETTINGS_LOCALNETWORK = "localNetwork";

bool SettingsHolder::ipv6() const
{
    Q_ASSERT(m_settings);
    if (!m_settings->contains(SETTINGS_IPV6)) {
        return SETTINGS_IPV6_DEFAULT;
    }

    return m_settings->value(SETTINGS_IPV6).toBool();
}

void SettingsHolder::setIpv6(bool ipv6)
{
    qDebug() << "Setting Ipv6:" << ipv6;

    Q_ASSERT(m_settings);
    m_settings->setValue(SETTINGS_IPV6, ipv6);
}

bool SettingsHolder::localNetwork() const
{
    Q_ASSERT(m_settings);
    if (!m_settings->contains(SETTINGS_LOCALNETWORK)) {
        return SETTINGS_LOCALNETWORK_DEFAULT;
    }

    return m_settings->value(SETTINGS_LOCALNETWORK).toBool();
}

void SettingsHolder::setLocalNetwork(bool localNetwork)
{
    qDebug() << "Setting LocalNetwork:" << localNetwork;

    Q_ASSERT(m_settings);
    m_settings->setValue(SETTINGS_LOCALNETWORK, localNetwork);
}
