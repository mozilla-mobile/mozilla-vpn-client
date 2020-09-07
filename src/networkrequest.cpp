#include "networkrequest.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

NetworkRequest::NetworkRequest(QObject *parent) : QObject(parent)
{
    qDebug() << "Network request created";

    QByteArray userAgent;
    userAgent.append("MozillaVPN/" APP_VERSION " (");
    userAgent.append(QSysInfo::productType());
    userAgent.append(" ");
    userAgent.append(QSysInfo::productVersion());
    userAgent.append(")");

    m_request.setRawHeader("User-Agent", userAgent);

    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkRequest::replyFinished);
    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkRequest::deleteLater);
}

// static
NetworkRequest *NetworkRequest::createForAuthenticate(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/login");
    r->m_request.setUrl(url);

    Q_ASSERT(r->m_manager);
    qDebug() << "Network starting: " << r;

    r->m_manager->post(r->m_request, QByteArray());

    return r;
}

// static
NetworkRequest *NetworkRequest::createForAuthenticationVerification(QObject *parent,
                                                                    const QString &verificationUrl)
{
    Q_ASSERT(parent);

    NetworkRequest *r = new NetworkRequest(parent);

    QUrl url(verificationUrl);
    r->m_request.setUrl(url);

    Q_ASSERT(r->m_manager);
    qDebug() << "Network starting: " << r;

    r->m_manager->get(r->m_request);
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
    authorizationHeader.append(vpn->token());
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
    qDebug() << "Network starting: " << json;

    r->m_manager->post(r->m_request, json.toJson());

    return r;
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(MozillaVPN *vpn, const QString &pubKey)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QString url(vpn->getApiUrl());
    url.append("/api/v1/vpn/device/");
    url.append(QUrl::toPercentEncoding(pubKey));

    QUrl u(url);
    Q_ASSERT(r->m_manager);
    r->m_request.setUrl(QUrl(url));
    qDebug() << "Network starting" << r->m_request.url();

    r->m_manager->sendCustomRequest(r->m_request, "DELETE");

    return r;
}

NetworkRequest *NetworkRequest::createForServers(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    NetworkRequest *r = new NetworkRequest(vpn);

    QByteArray authorizationHeader = "Bearer ";
    authorizationHeader.append(vpn->token());
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
    authorizationHeader.append(vpn->token());
    r->m_request.setRawHeader("Authorization", authorizationHeader);

    QUrl url(vpn->getApiUrl());
    url.setPath("/api/v1/vpn/account");
    r->m_request.setUrl(url);

    r->m_manager->get(r->m_request);

    return r;
}

void NetworkRequest::replyFinished(QNetworkReply *reply)
{
    Q_ASSERT(reply);
    Q_ASSERT(reply->isFinished());

    qDebug() << "Network reply received: " << reply;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply->error();
        emit requestFailed(reply->error());
        return;
    }

    QByteArray data = reply->readAll();
    emit requestCompleted(data);
}
