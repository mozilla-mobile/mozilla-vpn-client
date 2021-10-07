/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "timersingleshot.h"

#include <QDir>
#include <QFile>

namespace {
Logger logger(LOG_NETWORKING, "WASM NetworkRequest");

void createDummyRequest(NetworkRequest* r, const QString& resource) {
  TimerSingleShot::create(r, 200, [r, resource] {
    QByteArray data;

    if (!resource.isEmpty()) {
      QFile file(resource);
      if (!file.open(QFile::ReadOnly | QFile::Text)) {
        logger.error() << "Failed to open" << resource;
        return;
      }

      data = file.readAll();
      file.close();
    }

    emit r->requestCompleted(data.replace(
        "%%PUBLICKEY%%",
        MozillaVPN::instance()->keys()->publicKey().toLocal8Bit()));
  });
}

void createDummyRequest(NetworkRequest* r) { createDummyRequest(r, ""); }

}  // namespace

NetworkRequest::NetworkRequest(QObject* parent, int status,
                               bool setAuthorizationHeader)
    : QObject(parent), m_status(status) {
  Q_UNUSED(setAuthorizationHeader);

  MVPN_COUNT_CTOR(NetworkRequest);

  logger.debug() << "Network request created";
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

void NetworkRequest::abort() {}

// static
QString NetworkRequest::apiBaseUrl() {
  if (Constants::inProduction()) {
    return Constants::API_PRODUCTION_URL;
  }

  return Constants::getStagingServerAddress();
}

// static
NetworkRequest* NetworkRequest::createForGetUrl(QObject* parent,
                                                const QString& url,
                                                int status) {
  Q_ASSERT(parent);
  Q_UNUSED(url);

  NetworkRequest* r = new NetworkRequest(parent, status, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    QObject* parent, const QString&, const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);

  createDummyRequest(r, ":/networkrequests/authentication.json");
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(QObject* parent,
                                                        const QString&,
                                                        const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(QObject* parent,
                                                       const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 204, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/servers.json");
  return r;
}

NetworkRequest* NetworkRequest::createForServerExtra(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/account.json");
  return r;
}

// static
NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent,
                                                const QHostAddress&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/ipinfo.json");
  return r;
}

// static
NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl&, const QByteArray&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForHeartbeat(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForSurveyData(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFeedback(QObject* parent,
                                                  const QString& feedbackText,
                                                  const QString& logs,
                                                  const qint8 rating,
                                                  const QString& category) {
  Q_UNUSED(feedbackText);
  Q_UNUSED(logs);
  Q_UNUSED(rating);
  Q_UNUSED(category);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForSupportTicket(
    QObject* parent, const QString& email, const QString& subject,
    const QString& issueText, const QString& logs, const QString& category) {
  Q_UNUSED(parent);
  Q_UNUSED(email);
  Q_UNUSED(subject);
  Q_UNUSED(issueText);
  Q_UNUSED(logs);
  Q_UNUSED(category);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForGetFeatureList(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountStatus(
    QObject* parent, const QString& emailAddress) {
  Q_ASSERT(parent);
  Q_UNUSED(emailAddress);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAccountCreation(
    QObject* parent, const QString& email, const QByteArray& authpw,
    const QUrlQuery& query) {
  Q_ASSERT(parent);
  Q_UNUSED(email);
  Q_UNUSED(authpw);
  Q_UNUSED(query);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaLogin(QObject* parent,
                                                  const QString& email,
                                                  const QByteArray& authpw,
                                                  const QString& unblockCode,
                                                  const QUrlQuery& query) {
  Q_ASSERT(parent);
  Q_UNUSED(email);
  Q_UNUSED(authpw);
  Q_UNUSED(unblockCode);
  Q_UNUSED(query);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSendUnblockCode(
    QObject* parent, const QString& emailAddress) {
  Q_ASSERT(parent);
  Q_UNUSED(emailAddress);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByEmailCode(
    QObject* parent, const QByteArray& sessionToken, const QString& code,
    const QUrlQuery& query) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);
  Q_UNUSED(code);
  Q_UNUSED(query);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionVerifyByTotpCode(
    QObject* parent, const QByteArray& sessionToken, const QString& code,
    const QUrlQuery& query) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);
  Q_UNUSED(code);
  Q_UNUSED(query);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionResendCode(
    QObject* parent, const QByteArray& sessionToken) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaAuthz(
    QObject* parent, const QByteArray& sessionToken, const QUrlQuery& query) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);
  Q_UNUSED(query);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForFxaSessionDestroy(
    QObject* parent, const QByteArray& sessionToken) {
  Q_ASSERT(parent);
  Q_UNUSED(sessionToken);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForProducts(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

void NetworkRequest::replyFinished() {}

void NetworkRequest::timeout() {}

void NetworkRequest::getRequest() {}

void NetworkRequest::deleteRequest() {}

void NetworkRequest::postRequest(const QByteArray&) {}

void NetworkRequest::handleReply(QNetworkReply*) {}

int NetworkRequest::statusCode() const { return 200; }

void NetworkRequest::sslErrors(const QList<QSslError>& errors) {
  Q_UNUSED(errors);
}