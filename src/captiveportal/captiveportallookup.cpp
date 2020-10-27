/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportallookup.h"
#include "logger.h"

#include <QDnsLookup>
#include <QHostAddress>

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalLookup");
}

CaptivePortalLookup::CaptivePortalLookup(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, [this]() { abort(); });
}

void CaptivePortalLookup::start()
{
    logger.log() << "Captive portal lookup started";
    start(CAPTIVEPORTAL_HOST);
    m_timer.start(CAPTIVEPORTAL_LOOKUPTIMER);
}

void CaptivePortalLookup::start(const QString &host)
{
    logger.log() << "Captive portal lookup started:" << host << "lookups:" << m_lookups;

    // We have been aborted!
    if (m_lookups == -1) {
        return;
    }

    ++m_lookups;

    QDnsLookup *dnsLookup = new QDnsLookup(this);

    connect(dnsLookup, &QDnsLookup::finished, [this, dnsLookup]() { lookupCompleted(dnsLookup); });

    dnsLookup->setType(QDnsLookup::ANY);
    dnsLookup->setName(host);
    dnsLookup->lookup();
}

void CaptivePortalLookup::lookupCompleted(QDnsLookup *dnsLookup)
{
    logger.log() << "Captive portal lookup completed - lookups:" << m_lookups;

    // We have been aborted!
    if (m_lookups == -1) {
        return;
    }

    Q_ASSERT(m_lookups > 0);
    --m_lookups;

    // Check the lookup succeeded.
    if (dnsLookup->error() != QDnsLookup::NoError) {
        logger.log() << "DNS lookup failed";
        dnsLookup->deleteLater();
        maybeComplete();
        return;
    }

    // CNAME
    {
        const QList<QDnsDomainNameRecord> records = dnsLookup->canonicalNameRecords();
        logger.log() << "Found CNAMEs:" << records.length();
        for (const QDnsDomainNameRecord &record : records) {
            start(record.value());
        }
    }

    // A
    {
        const QList<QDnsHostAddressRecord> records = dnsLookup->hostAddressRecords();
        logger.log() << "Found As:" << records.length();
        for (const QDnsHostAddressRecord &record : records) {
            const QHostAddress &address = record.value();
            if (address.isBroadcast() || address.isLinkLocal() || address.isLoopback()
                || address.isMulticast() || address.isNull()) {
                continue;
            }

            if (address.protocol() == QAbstractSocket::IPv6Protocol) {
                m_data.addIpv6Address(address.toString());
                continue;
            }

            if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                m_data.addIpv4Address(address.toString());
                continue;
            }
        }
    }

    dnsLookup->deleteLater();
    maybeComplete();
}

void CaptivePortalLookup::maybeComplete()
{
    if (m_lookups > 0) {
        return;
    }

    emit completed(m_data);
    deleteLater();
}

void CaptivePortalLookup::abort()
{
    logger.log() << "Too much! Let's abort the lookup.";

    m_lookups = -1;
    maybeComplete();
}
