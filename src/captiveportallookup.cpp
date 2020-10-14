#include "captiveportallookup.h"

#include <QDnsLookup>
#include <QHostAddress>
#include <QtDebug>

constexpr int32_t CAPTIVEPORTAL_TIMER = 5000;

constexpr const char *CAPTIVEPORTAL_HOST = "detectportal.firefox.com";

CaptivePortalLookup::CaptivePortalLookup(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, [this]() { abort(); });
}

void CaptivePortalLookup::start()
{
    qDebug() << "Captive portal lookup started";
    start(CAPTIVEPORTAL_HOST);
    m_timer.start(CAPTIVEPORTAL_TIMER);
}

void CaptivePortalLookup::start(const QString &host)
{
    qDebug() << "Captive portal lookup started:" << host << "lookups:" << m_lookups;

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
    qDebug() << "Captive portal lookup completed - lookups:" << m_lookups;

    // We have been aborted!
    if (m_lookups == -1) {
        return;
    }

    Q_ASSERT(m_lookups > 0);
    --m_lookups;

    // Check the lookup succeeded.
    if (dnsLookup->error() != QDnsLookup::NoError) {
        qDebug() << "DNS lookup failed";
        dnsLookup->deleteLater();
        maybeComplete();
        return;
    }

    // CNAME
    {
        const QList<QDnsDomainNameRecord> records = dnsLookup->canonicalNameRecords();
        qDebug() << "Found CNAMEs:" << records.length();
        for (const QDnsDomainNameRecord &record : records) {
            start(record.value());
        }
    }

    // A
    {
        const QList<QDnsHostAddressRecord> records = dnsLookup->hostAddressRecords();
        qDebug() << "Found As:" << records.length();
        for (const QDnsHostAddressRecord &record : records) {
            const QHostAddress &address = record.value();
            if (address.isBroadcast() || address.isLinkLocal() || address.isLoopback()
                || address.isMulticast() || address.isNull()) {
                continue;
            }

            m_ips.append(record.value().toString());
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

    emit completed(m_ips);
    deleteLater();
}

void CaptivePortalLookup::abort()
{
    qDebug() << "Too much! Let's abort the lookup.";

    m_lookups = -1;
    maybeComplete();
}
