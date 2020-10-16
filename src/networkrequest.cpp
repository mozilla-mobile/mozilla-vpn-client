/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "captiveportal/captiveportal.h"
#include "logger.h"
#include "mozillavpn.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
Logger logger("NetworkRequest");
}

NetworkRequest::NetworkRequest(QObject *parent) : QObject(parent)
{
    logger.log() << "Network request created";

    QByteArray userAgent;
    userAgent.append("MozillaVPN/" APP_VERSION " (");
    userAgent.append(QSysInfo::productType().toLocal8Bit());
    userAgent.append(" ");
    userAgent.append(QSysInfo::productVersion().toLocal8Bit());
    userAgent.append(")");

    m_request.setRawHeader("User-Agent", userAgent);

    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkRequest::replyFinished);
    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkRequest::deleteLater);
}

// static
NetworkRequest *NetworkRequest::createForAuthenticationVerification(MozillaVPN *vpn,
                                                                    const QString &pkceCodeSuccess,
                                                                    const QString &pkceCodeVerifier)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v2/vpn/login/verify");
    r->m_request.setUrl(url);

    QJsonObject obj;
    obj.insert("code", pkceCodeSuccess);
    obj.insert("code_verifier", pkceCodeVerifier);

    QJsonDocument json;
    json.setObject(obj);

    Q_ASSERT(r->m_manager);
    r->m_manager->post(r->m_request, json.toJson());

    return r;
}

// static
NetworkRequest *NetworkRequest::createForDeviceCreation(MozillaVPN *vpn,
                                                        const QString &deviceName,
                                                        const QString &pubKey)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/device");
    r->m_request.setUrl(url);

    QJsonObject obj;
    obj.insert("name", deviceName);
    obj.insert("pubkey", pubKey);

    QJsonDocument json;
    json.setObject(obj);

    Q_ASSERT(r->m_manager);
    r->m_manager->post(r->m_request, json.toJson());

    return r;
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(MozillaVPN *vpn, const QString &pubKey)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QString url(vpn->getApiUrl());
    url.append("/api/v1/vpn/device/");
    url.append(QUrl::toPercentEncoding(pubKey));

    QUrl u(url);
    Q_ASSERT(r->m_manager);
    r->m_request.setUrl(QUrl(url));
    logger.log() << "Network starting" << r->m_request.url().toString();

    r->m_manager->sendCustomRequest(r->m_request, "DELETE");

    return r;
}

NetworkRequest *NetworkRequest::createForServers(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/servers");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}

NetworkRequest *NetworkRequest::createForVersions(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/versions");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}

NetworkRequest *NetworkRequest::createForAccount(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/account");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}

NetworkRequest *NetworkRequest::createForIpInfo(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/ipinfo");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}

NetworkRequest *NetworkRequest::createForCaptivePortalDetection(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QUrl url(CAPTIVEPORTAL_URL);
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);
    return r;
}

#ifdef IOS_INTEGRATION
NetworkRequest *NetworkRequest::createForIOSProducts(MozillaVPN* vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/products/ios");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}
#endif

void NetworkRequest::replyFinished(QNetworkReply *reply)
{
    Q_ASSERT(reply);
    Q_ASSERT(reply->isFinished());

    logger.log() << "Network reply received";

    if (reply->error() != QNetworkReply::NoError) {
        logger.log() << "Network error: " << reply->error() << "body: " << reply->readAll();
        emit requestFailed(reply->error());
        return;
    }

    QByteArray data = reply->readAll();
    emit requestCompleted(data);
}
