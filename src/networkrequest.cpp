/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "captiveportal/captiveportal.h"
#include "logger.h"
#include "networkmanager.h"
#include "settingsholder.h"

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
                                                                    const QString &pkceCodeSuccess,
                                                                    const QString &pkceCodeVerifier)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QUrl url(NetworkManager::instance()->apiUrl());
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
                                                        const QString &deviceName,
                                                        const QString &pubKey)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QUrl url(NetworkManager::instance()->apiUrl());
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
                                                       const QString &pubKey)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QString url(NetworkManager::instance()->apiUrl());
    url.append("/api/v1/vpn/device/");
    url.append(QUrl::toPercentEncoding(pubKey));

    QUrl u(url);
    r->m_request.setUrl(QUrl(url));
    logger.log() << "Network starting" << r->m_request.url().toString();

    r->deleteRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForServers(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/servers");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForVersions(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/versions");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForAccount(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/account");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForIpInfo(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
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

NetworkRequest *NetworkRequest::createForCaptivePortalLookup(QObject *parent)
{
    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/dns/detectportal");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

#ifdef MVPN_IOS
NetworkRequest *NetworkRequest::createForIOSProducts(QObject *parent)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/products/ios");
    r->m_request.setUrl(url);

    r->getRequest();
    return r;
}

NetworkRequest *NetworkRequest::createForIOSPurchase(QObject *parent,
                                                     const QString &receipt)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);
    r->m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(NetworkManager::instance()->apiUrl());
    url.setPath("/api/v1/vpn/purchases/ios");
    r->m_request.setUrl(url);

    QJsonObject obj;
    obj.insert("receipt", QJsonValue(receipt));
    obj.insert("appId", "org.mozilla.ios.FirefoxVPN");

    QJsonDocument json;
    json.setObject(obj);

    r->postRequest(json.toJson(QJsonDocument::Compact));
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
    QNetworkAccessManager *manager = NetworkManager::instance()->networkAccessManager();
    handleReply(manager->get(m_request));
}

void NetworkRequest::deleteRequest()
{
    QNetworkAccessManager *manager = NetworkManager::instance()->networkAccessManager();
    handleReply(manager->sendCustomRequest(m_request, "DELETE"));
}

void NetworkRequest::postRequest(const QByteArray &body)
{
    QNetworkAccessManager *manager = NetworkManager::instance()->networkAccessManager();
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
