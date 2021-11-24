/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectioncheck.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

constexpr uint32_t CONNECTION_CHECK_TIMEOUT_MSEC = 10000;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionCheck");
}

ConnectionCheck::ConnectionCheck() {
  MVPN_COUNT_CTOR(ConnectionCheck);

  connect(&m_timer, &QTimer::timeout, this, &ConnectionCheck::timeout);
  connect(&m_pingHelper, &PingHelper::pingSentAndReceived, this,
          &ConnectionCheck::pingSentAndReceived);

  m_timer.setSingleShot(true);
}

ConnectionCheck::~ConnectionCheck() { MVPN_COUNT_DTOR(ConnectionCheck); }

void ConnectionCheck::start() {
  logger.debug() << "Starting a connection check";
  MozillaVPN::instance().controller()->getStatus(
      [this](const QString& serverIpv4Gateway, const QString& deviceIpv4Address,
             uint64_t txBytes, uint64_t rxBytes) {
        Q_UNUSED(txBytes);
        Q_UNUSED(rxBytes);

        stop();

        if (serverIpv4Gateway.isEmpty()) {
          emit failure();
          return;
        }
        m_timer.start(CONNECTION_CHECK_TIMEOUT_MSEC);
        m_pingHelper.start(serverIpv4Gateway, deviceIpv4Address);
      });
}

void ConnectionCheck::stop() {
  logger.debug() << "Stopping a connection check";
  m_timer.stop();
  m_pingHelper.stop();
}

void ConnectionCheck::timeout() {
  logger.warning() << "Request timeout";
  emit failure();
}

void ConnectionCheck::pingSentAndReceived(qint64 msec) {
  Q_UNUSED(msec);

  logger.debug() << "Ping sent and received";

  if (m_timer.isActive()) {
    stop();
    emit success();
  }
}
