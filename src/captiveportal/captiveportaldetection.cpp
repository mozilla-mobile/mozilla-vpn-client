/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "captiveportalrequest.h"
#include "mozillavpn.h"

#include <QDebug>

CaptivePortalDetection::CaptivePortalDetection()
{
    connect(&m_timer, &QTimer::timeout, this, &CaptivePortalDetection::detectCaptivePortal);
}

void CaptivePortalDetection::controllerStateChanged()
{
    qDebug() << "Controller state changed";

    if (MozillaVPN::instance()->controller()->state() == Controller::StateOn) {
        m_timer.start(CAPTIVEPORTAL_REQUEST_TIMEOUT);
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

    CaptivePortalRequest *request = new CaptivePortalRequest(this);
    connect(request, &CaptivePortalRequest::completed, [this](bool detected) {
        qDebug() << "Captive portal request completed - detected:" << detected;

        if (!m_active) {
            qDebug() << "Disabled in the meantime.";
            return;
        }

        if (detected) {
            emit captivePortalDetected();
        }
    });

    request->run();
}
