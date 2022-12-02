/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "constants.h"
#include "helper.h"
#include "networkrequest.h"
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

// static
NetworkRequest* NetworkRequest::createForUploadDataHostAddress(
    Task* parent, const QString& url, QIODevice* uploadData,
    const QHostAddress& address) {
  Q_UNUSED(url);
  Q_UNUSED(uploadData);
  Q_UNUSED(address);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForGetHostAddress(
    Task* parent, const QString& url, const QHostAddress& address) {
  Q_ASSERT(parent);
  Q_UNUSED(url);
  Q_UNUSED(address);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForUploadData(Task* parent,
                                                    const QString& url,
                                                    QIODevice* uploadData) {
  Q_ASSERT(parent);
  Q_UNUSED(url);
  Q_UNUSED(uploadData);
  return new NetworkRequest(parent, 1234, false);
}

void NetworkRequest::abort() {}

int NetworkRequest::statusCode() const { return 200; }

// static
NetworkRequest* NetworkRequest::createForFxaSessionDestroy(
    Task* parent, const QByteArray& sessionToken) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountStatus(
    Task* parent, const QString& emailAddress) {
  Q_ASSERT(parent);
  Q_UNUSED(emailAddress);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaLogin(
    Task* parent, const QString& email, const QByteArray& authpw,
    const QString& originalLoginEmail, const QString& unblockCode,
    const QString& fxaClientId, const QString& fxaDeviceId,
    const QString& fxaFlowId, double fxaFlowBeginTime) {
  Q_UNUSED(email);
  Q_UNUSED(authpw);
  Q_UNUSED(originalLoginEmail);
  Q_UNUSED(unblockCode);
  Q_UNUSED(fxaClientId);
  Q_UNUSED(fxaDeviceId);
  Q_UNUSED(fxaFlowId);
  Q_UNUSED(fxaFlowBeginTime);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountCreation(
    Task* parent, const QString& email, const QByteArray& authpw,
    const QString& fxaClientId, const QString& fxaDeviceId,
    const QString& fxaFlowId, double fxaFlowBeginTime) {
  Q_UNUSED(email);
  Q_UNUSED(authpw);
  Q_UNUSED(fxaClientId);
  Q_UNUSED(fxaDeviceId);
  Q_UNUSED(fxaFlowId);
  Q_UNUSED(fxaFlowBeginTime);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaSendUnblockCode(
    Task* parent, const QString& emailAddress) {
  Q_UNUSED(emailAddress);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByEmailCode(
    Task* parent, const QByteArray& sessionToken, const QString& code,
    const QString& fxaClientId, const QString& fxaScope) {
  Q_UNUSED(sessionToken);
  Q_UNUSED(code);
  Q_UNUSED(fxaClientId);
  Q_UNUSED(fxaScope);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionResendCode(
    Task* parent, const QByteArray& sessionToken) {
  Q_UNUSED(sessionToken);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByTotpCode(
    Task* parent, const QByteArray& sessionToken, const QString& code,
    const QString& fxaClientId, const QString& fxaScope) {
  Q_UNUSED(sessionToken);
  Q_UNUSED(code);
  Q_UNUSED(fxaClientId);
  Q_UNUSED(fxaScope);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaTotpCreation(
    Task* parent, const QByteArray& sessionToken) {
  Q_UNUSED(sessionToken);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaAttachedClients(
    Task* parent, const QByteArray& sessionToken) {
  Q_UNUSED(sessionToken);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountDeletion(
    Task* parent, const QByteArray& sessionToken, const QString& emailAddress,
    const QByteArray& authpw) {
  Q_UNUSED(sessionToken);
  Q_UNUSED(emailAddress);
  Q_UNUSED(authpw);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForFxaAuthz(
    Task* parent, const QByteArray& sessionToken, const QString& fxaClientId,
    const QString& fxaState, const QString& fxaScope,
    const QString& fxaAccessType) {
  Q_UNUSED(sessionToken);
  Q_UNUSED(fxaClientId);
  Q_UNUSED(fxaState);
  Q_UNUSED(fxaScope);
  Q_UNUSED(fxaAccessType);
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForHeartbeat(Task* parent) {
  return new NetworkRequest(parent, 1234, false);
}

// static
NetworkRequest* NetworkRequest::createForAdjustProxy(
    Task* parent, const QString& method, const QString& path,
    const QList<QPair<QString, QString>>& headers,
    const QString& queryParameters, const QString& bodyParameters,
    const QList<QString>& unknownParameters) {
  Q_UNUSED(method);
  Q_UNUSED(path);
  Q_UNUSED(headers);
  Q_UNUSED(queryParameters);
  Q_UNUSED(bodyParameters);
  Q_UNUSED(unknownParameters);
  return new NetworkRequest(parent, 1234, false);
}
