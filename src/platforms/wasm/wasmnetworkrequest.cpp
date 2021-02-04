/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkrequest.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "timersingleshot.h"

#include <QDir>
#include <QFile>

namespace {
Logger logger(LOG_NETWORKING, "WASM NetworkRequest");

// A simple class to make QNetworkReply CTOR public
class NetworkReply : public QNetworkReply {
 public:
  NetworkReply() : QNetworkReply(nullptr) {}

  qint64 readData(char*, qint64) override { return -1; }
  void abort() override {}
};

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

    NetworkReply nr;
    emit r->requestCompleted(
        &nr, data.replace(
                 "%%PUBLICKEY%%",
                 MozillaVPN::instance()->keys()->publicKey().toLocal8Bit()));
  });
}

void createDummyRequest(NetworkRequest* r) { createDummyRequest(r, ""); }

}  // namespace

NetworkRequest::NetworkRequest(QObject* parent, int status)
    : QObject(parent), m_status(status) {
  MVPN_COUNT_CTOR(NetworkRequest);

  logger.log() << "Network request created";
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    QObject* parent, const QString&, const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);

  createDummyRequest(r, ":/networkrequests/authentication.json");
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(QObject* parent,
                                                        const QString&,
                                                        const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 201);
  createDummyRequest(r);
  return r;
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(QObject* parent,
                                                       const QString&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 204);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  createDummyRequest(r, ":/networkrequests/servers.json");
  return r;
}

NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  createDummyRequest(r, ":/networkrequests/account.json");
  return r;
}

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  createDummyRequest(r, ":/networkrequests/ipinfo.json");
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl&, const QByteArray&) {
  Q_ASSERT(parent);

  NetworkRequest* r = new NetworkRequest(parent, 200);
  createDummyRequest(r);
  return r;
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  NetworkRequest* r = new NetworkRequest(parent, 200);
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
