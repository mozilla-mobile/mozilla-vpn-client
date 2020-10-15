/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalactivator.h"
#include "captiveportalrequest.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QTimer>

// Let's try to detect the captive portal any X secs.
constexpr int CAPTIVEPORTAL_ACTIVATOR_TIMEOUT_MSEC = 2000;

CaptivePortalActivator::CaptivePortalActivator(QObject *parent) : QObject(parent) {}

void CaptivePortalActivator::run()
{
    qDebug() << "Captive Portal Activator";
    QTimer::singleShot(CAPTIVEPORTAL_ACTIVATOR_TIMEOUT_MSEC,
                       this,
                       &CaptivePortalActivator::checkStatus);
}

void CaptivePortalActivator::checkStatus()
{
    // We have changed state.
    if (MozillaVPN::instance()->controller()->state() != Controller::StateCaptivePortal) {
        deleteLater();
        return;
    }

    CaptivePortalRequest *request = new CaptivePortalRequest(this);
    connect(request, &CaptivePortalRequest::completed, [this](bool detected) {
        qDebug() << "Captive portal detection:" << detected;

        // We have changed state.
        if (MozillaVPN::instance()->controller()->state() != Controller::StateCaptivePortal) {
            deleteLater();
            return;
        }

        if (detected) {
            run();
            return;
        }

        // It seems that the captive-portal is gone. We can reactivate the VPN.

        deleteLater();
        MozillaVPN::instance()->controller()->activate();
    });

    request->run();
}
