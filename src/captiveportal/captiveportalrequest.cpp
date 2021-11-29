/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"
#include "captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "task.h"
#include "timersingleshot.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalRequest");
}

CaptivePortalRequest::CaptivePortalRequest(Task* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalRequest);
}

CaptivePortalRequest::~CaptivePortalRequest() {
  MVPN_COUNT_DTOR(CaptivePortalRequest);
}

void CaptivePortalRequest::run() {
  SettingsHolder* settings = SettingsHolder::instance();

  QStringList ipv4Addresses = settings->captivePortalIpv4Addresses();
  QStringList ipv6Addresses = settings->captivePortalIpv6Addresses();

  // We do not have IPs to check.
  if (ipv4Addresses.isEmpty() && ipv6Addresses.isEmpty()) {
    emit completed(NoPortal);
    return;
  }
  m_running = 0;
  m_success = 0;

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
  logger.debug() << "request:" << url.toString();

  NetworkRequest* request = NetworkRequest::createForCaptivePortalDetection(
      static_cast<Task*>(parent()), url, CAPTIVEPORTAL_HOST);

  connect(request, &NetworkRequest::requestRedirected,
          [this](NetworkRequest* request, const QUrl& url) {
            // In Case the Captive Portal request Redirects, we 100% have one.
            logger.info() << "Portal Detected -> Redirect to "
                          << url.toString();
            request->abort();
            onResult(PortalDetected);
          });
  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.warning() << "Captive portal request failed:" << error;
            onResult(Failure);
          });

  connect(
      request, &NetworkRequest::requestCompleted,
      [this, request](const QByteArray& data) {
        logger.debug() << "Captive portal request completed:" << data;
        // Usually, captive-portal pages do a redirect to an internal page.
        if (request->statusCode() != 200) {
          logger.debug() << "Captive portal detected. Expected 200, received:"
                         << request->statusCode();
          onResult(PortalDetected);
          return;
        }

        if (QString(data).trimmed() == CAPTIVEPORTAL_REQUEST_CONTENT) {
          logger.debug() << "No captive portal!";
          onResult(NoPortal);
          return;
        }

        logger.debug() << "Captive portal detected. Content does not match.";
        onResult(PortalDetected);
      });

  m_running++;
}

void CaptivePortalRequest::onResult(CaptivePortalResult portalDetected) {
  Q_ASSERT(m_running > 0);
  m_running--;
  if (portalDetected == NoPortal) {
    m_success++;
  }

  // If any request detects a portal, we can terminate immediately.
  if (portalDetected == PortalDetected) {
    deleteLater();
    emit completed(portalDetected);
    return;
  }

  // Otherwise, we are complete after all the workers have terminated.
  if (m_running == 0) {
    deleteLater();
    emit completed((m_success > 0) ? NoPortal : Failure);
    return;
  }
}
