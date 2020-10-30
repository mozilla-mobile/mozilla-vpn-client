/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOHREQUEST_H
#define DOHREQUEST_H

#include <QObject>
#include <QStringList>

class DOHRequest : public QObject
{
    Q_OBJECT
public:
    explicit DOHRequest(QObject *parent);

    void resolve(const QString &host);

signals:
    void completed(const QStringList &ipv4Addresses, const QStringList &ipv6Addresses);

private:
    void resolveInternal(const QString &host, uint32_t type);

    void parseResult(const QByteArray &data);

    void maybeCompleted(uint32_t type);

private:
    QStringList m_ipv4Addresses;
    QStringList m_ipv6Addresses;

    bool m_ipv4AddressesCompleted = false;
    bool m_ipv6AddressesCompleted = false;
};

#endif // DOHREQUEST_H
