/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"
#include "captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "timersingleshot.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalRequest");
}

CaptivePortalRequest::CaptivePortalRequest(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalRequest);
}

CaptivePortalRequest::~CaptivePortalRequest() {
  MVPN_COUNT_DTOR(CaptivePortalRequest);
}

void CaptivePortalRequest::run() {
  SettingsHolder* settings = SettingsHolder::instance();

  QStringList ipv4Addresses;
  if (settings->hasCaptivePortalIpv4Addresses()) {
    ipv4Addresses = settings->captivePortalIpv4Addresses();
  }

  QStringList ipv6Addresses;
#if 0
  // TODO https://github.com/mozilla-mobile/mozilla-vpn-client/issues/593
  // The captive portal detection doesn't work in ipv6 because we do not
  // support (yet) the IPAddress ipv6 filtering.
  if (settings->ipv6Enabled() && settings->hasCaptivePortalIpv6Addresses()) {
    ipv6Addresses = settings->captivePortalIpv6Addresses();
  }
#endif

  // We do not have IPs to check.
  if (ipv4Addresses.isEmpty() && ipv6Addresses.isEmpty()) {
    onResult(NoPortal);
    return;
  }

  // We do not care which request succeeds.
  // Let's make 1 request for any available IP addresses. The first one will
  // delete all the others.

  for (const QString& address : ipv4Addresses) {
    QUrl url(QString(CAPTIVEPORTAL_URL_IPV4).arg(address));
    createRequest(url);
  }

  for (const QString& address : ipv6Addresses) {
    QUrl url(QString(CAPTIVEPORTAL_URL_IPV6).arg(address));
    createRequest(url);
  }
}

void CaptivePortalRequest::createRequest(const QUrl& url) {
  logger.log() << "request:" << url.toString();

  NetworkRequest* request = NetworkRequest::createForCaptivePortalDetection(
      this, url, CAPTIVEPORTAL_HOST);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Captive portal request failed:" << error;
            onResult(Failure);
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, request](const QByteArray& data) {
            logger.log() << "Captive portal request completed:" << data;
            // Usually, captive-portal pages do a redirect to an internal page.
            if (request->statusCode() != 200) {
              logger.log() << "Captive portal detected. Expected 200, received:"
                           << request->statusCode();
              onResult(PortalDetected);
              return;
            }

            if (QString(data).trimmed() == CAPTIVEPORTAL_REQUEST_CONTENT) {
              logger.log() << "No captive portal!";
              onResult(NoPortal);
              return;
            }

            logger.log() << "Captive portal detected. Content does not match.";
            onResult(PortalDetected);
          });
}

void CaptivePortalRequest::onResult(CaptivePortalResult portalDetected) {
  if (m_completed) {
    return;
  }
  m_completed = true;
  deleteLater();
  emit completed(portalDetected);
}
