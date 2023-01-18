/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequest.h"

#include "appconstants.h"
#include "captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "task.h"

namespace {
Logger logger("CaptivePortalRequest");
}

CaptivePortalRequest::CaptivePortalRequest(Task* parent) : QObject(parent) {
  MZ_COUNT_CTOR(CaptivePortalRequest);
}

CaptivePortalRequest::~CaptivePortalRequest() {
  MZ_COUNT_DTOR(CaptivePortalRequest);
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
    QUrl url(AppConstants::captivePortalUrl().arg(address));
    createRequest(url);
  }

  for (const QString& address : ipv6Addresses) {
    QUrl url(
        AppConstants::captivePortalUrl().arg(QString("[%1]").arg(address)));
    createRequest(url);
  }
}

void CaptivePortalRequest::createRequest(const QUrl& url) {
  logger.debug() << "request:" << url.toString();

  NetworkRequest* request = new NetworkRequest(static_cast<Task*>(parent()));

  // This enables the QNetworkReply::redirected for every type of redirect.
  request->requestInternal().setAttribute(
      QNetworkRequest::RedirectPolicyAttribute,
      QNetworkRequest::UserVerifiedRedirectPolicy);
  request->requestInternal().setRawHeader("Host", CAPTIVEPORTAL_HOST);
  request->requestInternal().setPeerVerifyName(CAPTIVEPORTAL_HOST);
  request->get(url);

  connect(request, &NetworkRequest::requestRedirected, this,
          [this](NetworkRequest* request, const QUrl& url) {
            // In Case the Captive Portal request Redirects, we 100% have one.
            logger.info() << "Portal Detected -> Redirect to "
                          << logger.sensitive(url.toString());
            request->abort();
            onResult(PortalDetected);
          });
  connect(
      request, &NetworkRequest::requestFailed, this,
      [this, request](QNetworkReply::NetworkError error, const QByteArray&) {
        if (request->isAborted()) {
          return;
        }

        logger.warning() << "Captive portal request failed:" << error;
        onResult(Failure);
      });

  connect(
      request, &NetworkRequest::requestCompleted, this,
      [this, request](const QByteArray& data) {
        if (request->isAborted()) {
          return;
        }

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
