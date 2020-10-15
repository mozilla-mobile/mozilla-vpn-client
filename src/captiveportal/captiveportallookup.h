/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
