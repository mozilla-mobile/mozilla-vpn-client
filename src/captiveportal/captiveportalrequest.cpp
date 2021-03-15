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
    emit completed(false);
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

  ++m_pendingRequests;

  NetworkRequest* request = NetworkRequest::createForCaptivePortalDetection(
      this, url, CAPTIVEPORTAL_HOST);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
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
    deleteLater();

    emit completed(false);
  }
}
