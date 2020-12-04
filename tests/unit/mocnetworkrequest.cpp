/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/leakdetector.h"
#include "../../src/timersingleshot.h"
#include "helper.h"
#include "networkrequest.h"

namespace {};

NetworkRequest::NetworkRequest(QObject* parent, int status)
    : QObject(parent), m_status(status) {
  MVPN_COUNT_CTOR(NetworkRequest);

  Q_ASSERT(!TestHelper::networkConfig.isEmpty());
  TestHelper::NetworkConfig nc = TestHelper::networkConfig.takeFirst();

  TimerSingleShot::create(this, 0, [this, nc]() {
    deleteLater();
    if (nc.m_status == TestHelper::NetworkConfig::Failure) {
      emit requestFailed(QNetworkReply::NetworkError::HostNotFoundError);
    } else {
      Q_ASSERT(nc.m_status == TestHelper::NetworkConfig::Success);
      emit requestCompleted(nc.m_body);
    }
  });
}

NetworkRequest::~NetworkRequest() { MVPN_COUNT_DTOR(NetworkRequest); }

// static
NetworkRequest* NetworkRequest::createForAuthenticationVerification(
    QObject* parent, const QString&, const QString&) {
  return new NetworkRequest(parent, 1234);
}

// static
NetworkRequest* NetworkRequest::createForDeviceCreation(QObject* parent,
                                                        const QString&,
                                                        const QString&) {
  return new NetworkRequest(parent, 1234);
}

// static
NetworkRequest* NetworkRequest::createForDeviceRemoval(QObject* parent,
                                                       const QString&) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForServers(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForVersions(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForAccount(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForIpInfo(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForCaptivePortalDetection(
    QObject* parent, const QUrl&, const QByteArray&) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForCaptivePortalLookup(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

#ifdef MVPN_IOS
NetworkRequest* NetworkRequest::createForIOSProducts(QObject* parent) {
  return new NetworkRequest(parent, 1234);
}

NetworkRequest* NetworkRequest::createForIOSPurchase(QObject* parent,
                                                     const QString&) {
  return new NetworkRequest(parent, 1234);
}
#endif

void NetworkRequest::replyFinished() { QFAIL("Not called!"); }
