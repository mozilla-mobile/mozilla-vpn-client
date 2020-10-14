#ifndef CAPTIVEPORTAL_H
#define CAPTIVEPORTAL_H

#include <QStringList>

class CaptivePortal final
{
public:
    const QStringList &ipv4Addresses() const { return m_ipv4Addresses; }
    const QStringList &ipv6Addresses() const { return m_ipv6Addresses; }

    void addIpv4Address(const QString &address);
    void addIpv6Address(const QString &address);

private:
    QStringList m_ipv4Addresses;
    QStringList m_ipv6Addresses;
};

#endif // CAPTIVEPORTAL_H
