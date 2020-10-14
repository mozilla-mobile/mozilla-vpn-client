#ifndef CAPTIVEPORTALLOOKUP_H
#define CAPTIVEPORTALLOOKUP_H

#include <QObject>
#include <QStringList>
#include <QTimer>

class QDnsLookup;

class CaptivePortalLookup : public QObject
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
    void completed(const QStringList &ips);

private:
    QTimer m_timer;

    QStringList m_ips;

    int m_lookups = 0;
};

#endif // CAPTIVEPORTALLOOKUP_H
