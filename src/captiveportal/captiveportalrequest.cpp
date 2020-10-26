/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"
#include "captiveportal.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalRequest");
}

CaptivePortalRequest::CaptivePortalRequest(QObject *parent) : QObject(parent)
{
}

void CaptivePortalRequest::run()
{
    NetworkRequest *request = NetworkRequest::createForCaptivePortalDetection(this);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        logger.log() << "Captive portal request failed:" << error;
        emit completed(false);
        deleteLater();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const int &status, const QByteArray &data) {
        if (status == 200) {
            logger.log() << "Captive portal request completed:" << data;

            deleteLater();

            if (QString(data).trimmed() == CAPTIVEPORTAL_REQUEST_CONTENT) {
                logger.log() << "No captive portal!";
                emit completed(false);
                return;
            }

            logger.log() << "Captive portal detected!";
            emit completed(true);
        } else {
            logger.logNon200Reply(status, data);
            emit completed(false);
            return;
        }
    });
}
