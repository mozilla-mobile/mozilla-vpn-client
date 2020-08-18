#include "taskauthenticationverifier.h"
#include "networkrequest.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

TaskAuthenticationVerifier::TaskAuthenticationVerifier(QObject *parent,
                                                       const QString &verificationUrl,
                                                       uint32_t pollInterval)
    : QObject(parent), m_verificationUrl(verificationUrl), m_pollInterval(pollInterval)
{
    qDebug() << "Authentication verifier created";

    run();
}

void TaskAuthenticationVerifier::run()
{
    QTimer::singleShot(1000 * m_pollInterval, this, &TaskAuthenticationVerifier::timerExpired);
}

void TaskAuthenticationVerifier::timerExpired()
{
    qDebug() << "Checking validation URL: " << m_verificationUrl;

    NetworkRequest *request = NetworkRequest::createForAuthenticationVerification(this,
                                                                                  m_verificationUrl);
    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Network request failed: " << error;
        run();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "Network completed: " << this << data;
        emit completed(data);
    });
}
