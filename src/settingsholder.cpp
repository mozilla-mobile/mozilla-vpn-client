#include "settingsholder.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QSettings>

constexpr bool SETTINGS_IPV6_DEFAULT = true;

bool SettingsHolder::ipv6() const
{
    Q_ASSERT(m_settings);
    if (!m_settings->contains("ipv6")) {
        return SETTINGS_IPV6_DEFAULT;
    }

    return m_settings->value("ipv6").toBool();
}

void SettingsHolder::setIpv6(bool ipv6)
{
    qDebug() << "Setting Ipv6:" << ipv6;

    Q_ASSERT(m_settings);
    m_settings->setValue("ipv6", ipv6);
}
