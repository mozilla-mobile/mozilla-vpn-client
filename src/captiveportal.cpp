#include "captiveportal.h"

void CaptivePortal::addIpv4Address(const QString &address)
{
    m_ipv4Addresses.append(address);
}

void CaptivePortal::addIpv6Address(const QString &address)
{
    m_ipv6Addresses.append(address);
}
