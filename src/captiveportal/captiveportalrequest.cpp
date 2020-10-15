/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"
#include "captiveportal.h"
#include "networkrequest.h"

CaptivePortalRequest::CaptivePortalRequest(QObject *parent) : QObject(parent)
{
}

void CaptivePortalRequest::run()
{
    NetworkRequest *request = NetworkRequest::createForCaptivePortalDetection(this);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Captive portal request failed:" << error;
        emit completed(false);
        deleteLater();
    });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &data) {
        qDebug() << "Captive portal request completed:" << data;

        deleteLater();

        if (QString(data).trimmed() == CAPTIVEPORTAL_REQUEST_CONTENT) {
            qDebug() << "No captive portal!";
            emit completed(false);
            return;
        }

        qDebug() << "Captive portal detected!";
        emit completed(true);
    });
}
