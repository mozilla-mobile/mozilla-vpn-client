/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

class MozillaVPN;
class QNetworkAccessManager;

class NetworkRequest final : public QObject
{
    Q_OBJECT

public:
    ~NetworkRequest() = default;

    // This object deletes itself at the end of the operation.

    static NetworkRequest *createForAuthenticationVerification(MozillaVPN *vpn,
                                                               const QString &pkceCodeSuccess,
                                                               const QString &pkceCodeVerifier);

    static NetworkRequest *createForDeviceCreation(MozillaVPN *vpn,
                                                   const QString &deviceName,
                                                   const QString &pubKey);

    static NetworkRequest *createForDeviceRemoval(MozillaVPN *vpn, const QString &pubKey);

    static NetworkRequest *createForServers(MozillaVPN *vpn);

    static NetworkRequest *createForAccount(MozillaVPN *vpn);

    static NetworkRequest *createForVersions(MozillaVPN *vpn);

    static NetworkRequest *createForIpInfo(MozillaVPN *vpn);

    static NetworkRequest *createForCaptivePortalDetection(QObject *parent);

private:
    NetworkRequest(QObject *parent);

private Q_SLOTS:
    void replyFinished(QNetworkReply *reply);

signals:
    void requestFailed(QNetworkReply::NetworkError error);
    void requestCompleted(const QByteArray &data);

private:
    QNetworkAccessManager *m_manager;
    QNetworkRequest m_request;
};

#endif // NETWORKREQUEST_H
