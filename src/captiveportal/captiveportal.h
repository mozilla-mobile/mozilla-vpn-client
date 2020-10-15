/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTAL_H
#define CAPTIVEPORTAL_H

#include <QStringList>

constexpr int32_t CAPTIVEPORTAL_LOOKUPTIMER = 5000;

constexpr const char *CAPTIVEPORTAL_HOST = "detectportal.firefox.com";
constexpr const char *CAPTIVEPORTAL_URL = "http://detectportal.firefox.com/success.txt";

constexpr int CAPTIVEPORTAL_REQUEST_TIMEOUT = 10000;
constexpr const char *CAPTIVEPORTAL_REQUEST_CONTENT = "success";

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
