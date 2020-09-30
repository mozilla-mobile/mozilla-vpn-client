/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERSFETCHER_H
#define SERVERSFETCHER_H

#include <QObject>
#include <QNetworkReply>

class MozillaVPN;

class ServersFetcher final : public QObject
{
    Q_OBJECT

public:
    ServersFetcher(QObject *parent) : QObject(parent) {}

    void run(MozillaVPN *vpn);

signals:
    void completed(const QByteArray& serverData);
    void failed(QNetworkReply::NetworkError error);
};

#endif // SERVERSFETCHER_H
