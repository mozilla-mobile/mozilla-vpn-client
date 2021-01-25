/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"

// In seconds, the timeout for unstable pings.
constexpr uint32_t PING_TIME_UNSTABLE_SEC = 1;

// In seconds, the timeout to detect no-signal pings.
constexpr uint32_t PING_TIME_NOSIGNAL_SEC = 3;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionHealth");
}

ConnectionHealth::ConnectionHealth() {
  MVPN_COUNT_CTOR(ConnectionHealth);

  connect(&m_noSignalTimer, &QTimer::timeout, this,
          &ConnectionHealth::noSignalDetected);
  m_noSignalTimer.setSingleShot(true);

  connect(&m_pingHelper, &PingHelper::pingSentAndReceived, this,
          &ConnectionHealth::pingSentAndReceived);
}

ConnectionHealth::~ConnectionHealth() { MVPN_COUNT_DTOR(ConnectionHealth); }

void ConnectionHealth::stop() {
  logger.log() << "ConnectionHealth deactivated";

  m_pingHelper.stop();
  m_noSignalTimer.stop();

  setStability(Stable);
}

void ConnectionHealth::setStability(ConnectionStability stability) {
  if (m_stability == stability) {
    return;
  }

  logger.log() << "Stability changed:" << stability;

  m_stability = stability;
  emit stabilityChanged();
}

void ConnectionHealth::connectionStateChanged() {
  logger.log() << "Connection state changed";

  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    stop();
    return;
  }

  MozillaVPN::instance()->controller()->getStatus(
      [this](const QString& serverIpv4Gateway, uint64_t txBytes,
             uint64_t rxBytes) {
        Q_UNUSED(txBytes);
        Q_UNUSED(rxBytes);

        stop();

        if (!serverIpv4Gateway.isEmpty() &&
            MozillaVPN::instance()->controller()->state() ==
                Controller::StateOn) {
          m_pingHelper.start(serverIpv4Gateway);
          m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
        }
      });
}

void ConnectionHealth::pingSentAndReceived(qint64 msec) {
  logger.log() << "Ping answer received in msec:" << msec;

  // If a ping has been received, we have signal. Restart the timer.
  m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);

  if (msec < PING_TIME_UNSTABLE_SEC * 1000) {
    setStability(Stable);
  } else {
    setStability(Unstable);
  }
}

void ConnectionHealth::noSignalDetected() {
  logger.log() << "No signal detected";
  setStability(NoSignal);
}
