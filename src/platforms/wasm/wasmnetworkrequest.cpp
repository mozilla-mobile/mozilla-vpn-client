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
        logger.log() << "Failed to open" << resource;
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

  logger.log() << "Network request created";
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

// static
QString NetworkRequest::apiBaseUrl() { return QString(Constants::API_URL); }

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

NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/servers.json");
  return r;
}

NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/account.json");
  return r;
}

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent,
                                                const QHostAddress&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r, ":/networkrequests/ipinfo.json");
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl&, const QByteArray&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForHeartbeat(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForSurveyData(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200, false);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForFeedback(QObject* parent,
                                                  const QString& feedbackText,
                                                  const QString& logs,
                                                  const qint8 rating,
                                                  const QString& category) {
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
