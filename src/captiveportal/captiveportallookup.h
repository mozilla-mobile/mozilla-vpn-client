#ifndef CAPTIVEPORTALLOOKUP_H
#define CAPTIVEPORTALLOOKUP_H

#include "captiveportal.h"

#include <QObject>
#include <QTimer>

class QDnsLookup;

class CaptivePortalLookup final : public QObject
{
    Q_OBJECT

public:
    explicit CaptivePortalLookup(QObject *parent);

    void start();

private:
    void start(const QString &host);

    void lookupCompleted(QDnsLookup *dnsLookup);

    void maybeComplete();

    void abort();

signals:
    void completed(const CaptivePortal &cp);

private:
    QTimer m_timer;

    CaptivePortal m_data;

    int m_lookups = 0;
};

#endif // CAPTIVEPORTALLOOKUP_H
