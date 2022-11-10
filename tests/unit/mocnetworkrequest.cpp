/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "helper.h"
#include "networkrequest.h"
#include "constants.h"
#include "settingsholder.h"
#include "task.h"

NetworkRequest::NetworkRequest(Task* parent, int status,
                               bool setAuthorizationHeader)
    : QObject(parent), m_expectedStatusCode(status) {
  Q_UNUSED(setAuthorizationHeader);

  MVPN_COUNT_CTOR(NetworkRequest);

  Q_ASSERT(!TestHelper::networkConfig.isEmpty());
  TestHelper::NetworkConfig nc = TestHelper::networkConfig.takeFirst();

  QTimer::singleShot(0, this, [this, nc]() {
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
  return Constants::envOrDefault("MVPN_API_BASE_URL",
                                 Constants::API_PRODUCTION_URL);
}

// static
NetworkRequest* NetworkRequest::createForGetUrl(Task* parent, const QString&,
                                                int status) {
  return new NetworkRequest(parent, status, false);
}

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    Task* parent, const QString&, const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(Task* parent,
                                                        const QString&,
                                                        const QString&,
                                                        const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(Task* parent,
                                                       const QString&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForServers(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForVersions(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForAccount(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForGetSubscriptionDetails(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForIpInfo(Task* parent,
                                                const QHostAddress&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    Task* parent, const QUrl&, const QByteArray&) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

NetworkRequest* NetworkRequest::createForProducts(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

#ifdef MVPN_IOS
NetworkRequest* NetworkRequest::createForIOSPurchase(Task* parent,
                                                     const QString&) {
  return new NetworkRequest(parent, 1234, false);
}
#endif

NetworkRequest* NetworkRequest::createForFeedback(Task* parent,
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
    Task* parent, const QString& email, const QString& subject,
    const QString& issueText, const QString& logs, const QString& category) {
  Q_UNUSED(email);
  Q_UNUSED(subject);
  Q_UNUSED(issueText);
  Q_UNUSED(logs);
  Q_UNUSED(category);

  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForGetFeatureList(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

void NetworkRequest::replyFinished() { QFAIL("Not called!"); }

void NetworkRequest::timeout() {}

#ifndef QT_NO_SSL
void NetworkRequest::sslErrors(const QList<QSslError>& errors) {
  Q_UNUSED(errors);
}
#endif

void NetworkRequest::disableTimeout() {}
