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

// Timeout for the network requests.
constexpr uint32_t REQUEST_TIMEOUT_MSEC = 15000;

namespace {
Logger logger(LOG_NETWORKING, "NetworkRequest");
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
    m_request.setTransferTimeout(REQUEST_TIMEOUT_MSEC);
}

// static
NetworkRequest *NetworkRequest::createForAuthenticationVerification(QObject *parent,
                                                                    MozillaVPN *vpn,
                                                                    const QString &pkceCodeSuccess,
                                                                    const QString &pkceCodeVerifier)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v2/vpn/login/verify");
    r->m_request.setUrl(url);

    QJsonObject obj;
    obj.insert("code", pkceCodeSuccess);
    obj.insert("code_verifier", pkceCodeVerifier);

    QJsonDocument json;
    json.setObject(obj);

    r->postRequest(json.toJson(QJsonDocument::Compact));
    return r;
}

// static
NetworkRequest *NetworkRequest::createForDeviceCreation(QObject *parent,
                                                        MozillaVPN *vpn,
                                                        const QString &deviceName,
                                                        const QString &pubKey)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

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

    r->postRequest(json.toJson(QJsonDocument::Compact));
    return r;
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(QObject *parent,
                                                       MozillaVPN *vpn,
                                                       const QString &pubKey)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QString url(vpn->getApiUrl());
    url.append("/api/v1/vpn/device/");
    url.append(QUrl::toPercentEncoding(pubKey));

    QUrl u(url);
    r->m_request.setUrl(QUrl(url));
    logger.log() << "Network starting" << r->m_request.url().toString();

    r->deleteRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForServers(QObject *parent, MozillaVPN *vpn)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/servers");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForVersions(QObject *parent, MozillaVPN *vpn)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/versions");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForAccount(QObject *parent, MozillaVPN *vpn)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/account");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForIpInfo(QObject *parent, MozillaVPN *vpn)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/ipinfo");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForCaptivePortalDetection(QObject *parent,
                                                                const QUrl &url,
                                                                const QByteArray &host)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    r->m_request.setUrl(url);
    r->m_request.setRawHeader("Host", host);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForDOH(QObject *parent,
                                             const QUrl &dohUrl,
                                             const QByteArray &dohHost)
{
    NetworkRequest *r = new NetworkRequest(parent);

    r->m_request.setUrl(dohUrl);
    r->m_request.setRawHeader("Host", dohHost);
    r->m_request.setRawHeader("Accept", "application/dns-json");

    r->getRequest();
    return r;
}

#ifdef IOS_INTEGRATION
NetworkRequest *NetworkRequest::createForIOSProducts(QObject *parent, MozillaVPN *vpn)
{
    Q_ASSERT(parent);
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

NetworkRequest *NetworkRequest::createForIOSPurchase(QObject *parent,
                                                     MozillaVPN *vpn,
                                                     const QString &orderId)
{
    Q_ASSERT(parent);
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/purchases/ios");
    r->m_request.setUrl(url);

    QJsonObject obj;
    obj.insert("receipt", QJsonValue(orderId));

    QJsonDocument json;
    json.setObject(obj);

    r->m_manager->post(r->m_request, json.toJson(QJsonDocument::Compact));

    return r;
}
#endif

void NetworkRequest::replyFinished()
{
    Q_ASSERT(m_reply);
    Q_ASSERT(m_reply->isFinished());

    logger.log() << "Network reply received";

    if (m_reply->error() != QNetworkReply::NoError) {
        logger.log() << "Network error: " << m_reply->error() << "body: " << m_reply->readAll();
        emit requestFailed(m_reply->error());
        return;
    }

    QByteArray data = m_reply->readAll();
    emit requestCompleted(data);
}

void NetworkRequest::getRequest()
{
    QNetworkAccessManager *manager = MozillaVPN::instance()->networkAccessManager();
    handleReply(manager->get(m_request));
}

void NetworkRequest::deleteRequest()
{
    QNetworkAccessManager *manager = MozillaVPN::instance()->networkAccessManager();
    handleReply(manager->sendCustomRequest(m_request, "DELETE"));
}

void NetworkRequest::postRequest(const QByteArray &body)
{
    QNetworkAccessManager *manager = MozillaVPN::instance()->networkAccessManager();
    handleReply(manager->post(m_request, body));
}

void NetworkRequest::handleReply(QNetworkReply *reply)
{
    Q_ASSERT(reply);
    Q_ASSERT(!m_reply);

    m_reply = reply;
    m_reply->setParent(this);

    connect(m_reply, &QNetworkReply::finished, this, &NetworkRequest::replyFinished);
    connect(m_reply, &QNetworkReply::finished, this, &QObject::deleteLater);
}
