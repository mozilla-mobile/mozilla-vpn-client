/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "captiveportalrequest.h"
#include "logger.h"
#include "mozillavpn.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection()
{
    connect(&m_timer, &QTimer::timeout, this, &CaptivePortalDetection::detectCaptivePortal);
}

void CaptivePortalDetection::initialize()
{
    m_active = MozillaVPN::instance()->settingsHolder()->captivePortalAlert();
}

void CaptivePortalDetection::controllerStateChanged()
{
    logger.log() << "Controller state changed";

    if (MozillaVPN::instance()->controller()->state() == Controller::StateOn) {
        m_timer.start(CAPTIVEPORTAL_REQUEST_TIMEOUT);
        detectCaptivePortal();
    } else {
        m_timer.stop();
    }
}

void CaptivePortalDetection::settingsChanged()
{
    logger.log() << "Settings has changed";
    m_active = MozillaVPN::instance()->settingsHolder()->captivePortalAlert();
}

void CaptivePortalDetection::detectCaptivePortal()
{
    logger.log() << "Detecting captive portal - status:" << m_active;

    if (!m_active) {
        return;
    }

    CaptivePortalRequest *request = new CaptivePortalRequest(this);
    connect(request, &CaptivePortalRequest::completed, [this](bool detected) {
        logger.log() << "Captive portal request completed - detected:" << detected;

        if (!m_active) {
            logger.log() << "Disabled in the meantime.";
            return;
        }

        // Comment this out to see the captive portal view each time.
        if (!detected) {
            return;
        }

        emit captivePortalDetected();
    });

    request->run();
}
