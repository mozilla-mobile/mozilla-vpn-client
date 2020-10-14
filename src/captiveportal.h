#ifndef CAPTIVEPORTAL_H
#define CAPTIVEPORTAL_H

#include <QStringList>

class CaptivePortal
{
public:
    void addIpv4Address(const QString &address);
    void addIpv6Address(const QString &address);

private:
    QStringList m_ipv4Addresses;
    QStringList m_ipv6Addresses;
};

#endif // CAPTIVEPORTAL_H
