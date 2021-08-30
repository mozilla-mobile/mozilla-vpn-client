/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "../../src/timersingleshot.h"
#include "helper.h"
#include "networkrequest.h"
#include "constants.h"

namespace {};

NetworkRequest::NetworkRequest(QObject* parent, int status,
                               bool setAuthorizationHeader)
    : QObject(parent), m_status(status) {
  Q_UNUSED(setAuthorizationHeader);

  MVPN_COUNT_CTOR(NetworkRequest);

  Q_ASSERT(!TestHelper::networkConfig.isEmpty());
  TestHelper::NetworkConfig nc = TestHelper::networkConfig.takeFirst();

  TimerSingleShot::create(this, 0, [this, nc]() {
    deleteLater();

    if (nc.m_status == TestHelper::NetworkConfig::Failure) {
      emit requestFailed(QNetworkReply::NetworkError::HostNotFoundError, "");
    } else {
      Q_ASSERT(nc.m_status == TestHelper::NetworkConfig::Success);

      emit requestCompleted(nc.m_body);
    }
  });
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

// static
QString NetworkRequest::apiBaseUrl() {
  return QString(Constants::API_STAGING_URL);
}

// static
NetworkRequest* NetworkRequest::createForGetUrl(QObject* parent, const QString&,
                                                int status) {
  return new NetworkRequest(parent, status, false);
}

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    QObject* parent, const QString&, const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(QObject* parent,
                                                        const QString&,
                                                        const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(QObject* parent,
                                                       const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent,
                                                const QHostAddress&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl&, const QByteArray&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForProducts(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

#ifdef MVPN_IOS
NetworkRequest* NetworkRequest::createForIOSPurchase(QObject* parent,
                                                     const QString&) {
  return new NetworkRequest(parent, 1234, false);
}
#endif

NetworkRequest* NetworkRequest::createForSurveyData(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForFeedback(QObject* parent,
                                                  const QString& feedbackText,
                                                  const QString& logs,
                                                  const qint8 rating,
                                                  const QString& category) {
  Q_UNUSED(feedbackText);
  Q_UNUSED(logs);
  Q_UNUSED(rating);
  Q_UNUSED(category);

  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForSupportTicket(
    QObject* parent, const QString& email, const QString& subject,
    const QString& issueText, const QString& logs, const QString& category) {
  Q_UNUSED(parent);
  Q_UNUSED(email);
  Q_UNUSED(subject);
  Q_UNUSED(issueText);
  Q_UNUSED(logs);
  Q_UNUSED(category);

  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForGetFeatureList(QObject* parent) {
  return new NetworkRequest(parent, 1234, false);
}

void NetworkRequest::replyFinished() { QFAIL("Not called!"); }

void NetworkRequest::timeout() {}
