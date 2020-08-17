#include "networkrequest.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

NetworkRequest::NetworkRequest(QObject *parent) : QObject(parent)
{
    qDebug() << "Network request created";

    m_request.setRawHeader("User-Agent", "MozillaVPN " APP_VERSION);

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
