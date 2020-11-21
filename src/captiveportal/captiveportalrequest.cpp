/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"
#include "captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalRequest");
}

CaptivePortalRequest::CaptivePortalRequest(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalRequest);
}

CaptivePortalRequest::~CaptivePortalRequest() {
  MVPN_COUNT_DTOR(CaptivePortalRequest);
}

void CaptivePortalRequest::createRequest(const QUrl& url) {
  logger.log() << "request:" << url.toString();

  ++m_pendingRequests;

  NetworkRequest* request = NetworkRequest::createForCaptivePortalDetection(
      this, url, CAPTIVEPORTAL_HOST);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error) {
            logger.log() << "Captive portal request failed:" << error;
            --m_pendingRequests;
            maybeComplete();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.log() << "Captive portal request completed:" << data;

            --m_pendingRequests;
            m_completed = true;
            deleteLater();

            if (QString(data).trimmed() == CAPTIVEPORTAL_REQUEST_CONTENT) {
              logger.log() << "No captive portal!";
              emit completed(false);
              return;
            }

            logger.log() << "Captive portal detected!";
            emit completed(true);
          });
}

void CaptivePortalRequest::maybeComplete() {
  logger.log() << "Failure - pendingRequests:" << m_pendingRequests;

  if (!m_completed && m_pendingRequests == 0) {
    m_completed = true;
    emit completed(false);
    deleteLater();
  }
}
