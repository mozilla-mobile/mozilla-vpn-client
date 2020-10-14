/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>

constexpr int CAPTIVE_PORTAL_TIMEOUT = 10000;
constexpr const char *CAPTIVE_PORTAL_CONTENT = "success";

CaptivePortalDetection::CaptivePortalDetection()
{
    connect(&m_timer, &QTimer::timeout, this, &CaptivePortalDetection::detectCaptivePortal);
}

void CaptivePortalDetection::controllerStateChanged()
{
    qDebug() << "Controller state changed";

    if (MozillaVPN::instance()->controller()->state() == Controller::StateOn) {
        m_timer.start(CAPTIVE_PORTAL_TIMEOUT);
        detectCaptivePortal();
    } else {
        m_timer.stop();
    }
}

void CaptivePortalDetection::settingsChanged()
{
    qDebug() << "Settings has changed";
    m_active = MozillaVPN::instance()->settingsHolder()->captivePortalAlert();
}

void CaptivePortalDetection::detectCaptivePortal()
{
    qDebug() << "Detecting captive portal - status:" << m_active;

    if (!m_active) {
        return;
    }

    NetworkRequest *request = NetworkRequest::createForCaptivePortalDetection(this);

    connect(request, &NetworkRequest::requestFailed, [](QNetworkReply::NetworkError error) {
        qDebug() << "Captive portal request failed:" << error;
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "Captive portal request completed:" << data;

        if (!m_active) {
            qDebug() << "Disabled in the meantime.";
            return;
        }

        if (QString(data).trimmed() == CAPTIVE_PORTAL_CONTENT) {
            qDebug() << "No captive portal!";
            return;
        }

        emit captivePortalDetected();
    });
}

