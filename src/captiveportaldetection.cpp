#include "captiveportaldetection.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>

constexpr int CAPTIVE_PORTAL_TIMEOUT = 10000;
constexpr const char *CAPTIVE_PORTAL_CONTENT = "success";

CaptivePortalDetection::CaptivePortalDetection()
{
    connect(&m_timer, &QTimer::timeout, [this]() {
        if (!m_active) {
            return;
        }

        handleFailure();
        stop();
    });
}

void CaptivePortalDetection::start()
{
    qDebug() << "CaptivePortalDetection start";
    m_active = true;

    m_timer.start(CAPTIVE_PORTAL_TIMEOUT);

    NetworkRequest *request = NetworkRequest::createForCaptivePortalDetection(this);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Captive portal request failed:" << error;

        if (!m_active) {
            qDebug() << "Disabled in the meantime.";
            return;
        }

        handleFailure();
        stop();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "Captive portal request completed:" << data;

        if (!m_active) {
            qDebug() << "Disabled in the meantime.";
            return;
        }

        if (QString(data).trimmed() == CAPTIVE_PORTAL_CONTENT) {
            handleSuccess();
        } else {
            handleFailure();
        }

        stop();
    });
}

void CaptivePortalDetection::stop()
{
    qDebug() << "CaptivePortalDetection stop";
    m_active = false;
    m_timer.stop();
}

void CaptivePortalDetection::handleFailure()
{
    qDebug() << "Captive portal detected!";
    emit captivePortalDetected();
}

void CaptivePortalDetection::handleSuccess()
{
    qDebug() << "No captive-portal detected!";
}
