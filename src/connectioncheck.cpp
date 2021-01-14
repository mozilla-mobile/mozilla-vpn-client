/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectioncheck.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

constexpr const uint32_t CONNECTION_CHECK_TIMEOUT_MSEC = 10000;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionCheck");
}

ConnectionCheck::ConnectionCheck() {
  MVPN_COUNT_CTOR(ConnectionCheck);

  connect(&m_timer, &QTimer::timeout, this, &ConnectionCheck::timeout);

  m_timer.setSingleShot(true);
}

ConnectionCheck::~ConnectionCheck() { MVPN_COUNT_DTOR(ConnectionCheck); }

void ConnectionCheck::start() {
  logger.log() << "Starting a connection check";

  if (m_networkRequest) {
    delete m_networkRequest;
  }

  m_networkRequest = NetworkRequest::createForConnectionCheck(this);

  connect(m_networkRequest, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Failed to check the connection" << error;
            m_networkRequest = nullptr;
            m_timer.stop();
            emit failure();
          });

  connect(m_networkRequest, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.log() << "Connection succeeded. Data:" << data;
            m_networkRequest = nullptr;
            m_timer.stop();
            emit success();
          });

  m_timer.start(CONNECTION_CHECK_TIMEOUT_MSEC);
}

void ConnectionCheck::stop() {
  logger.log() << "Stopping a connection check";

  if (m_networkRequest) {
    delete m_networkRequest;
    m_networkRequest = nullptr;
  }

  m_timer.stop();
}

void ConnectionCheck::timeout() {
  logger.log() << "Request timeout";

  Q_ASSERT(m_networkRequest);
  delete m_networkRequest;
  m_networkRequest = nullptr;

  emit failure();
}
