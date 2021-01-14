/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectioncheck.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

struct Step {
  uint32_t m_msec;
};

// We try with 3 requests with different timeout intervals. Each value here is
// in msecs.
Step s_steps[] = {
    {2000},
    {3000},
    {5000},
    // Sentinel
    {0},
};

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
            maybeTryAgain();
          });

  connect(m_networkRequest, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.log() << "Connection succeded. Data:" << data;
            m_networkRequest = nullptr;
            m_timer.stop();
            emit success();
          });

  m_timer.start(s_steps[m_step].m_msec);
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

void ConnectionCheck::maybeTryAgain() {
  logger.log() << "Current retry is:" << m_step;
  ++m_step;

  if (s_steps[m_step].m_msec == 0) {
    logger.log() << "No extra try. Let's fail.";
    emit failure();
    return;
  }

  logger.log() << "Let's try again with" << s_steps[m_step].m_msec << "msecs";
  start();
}
