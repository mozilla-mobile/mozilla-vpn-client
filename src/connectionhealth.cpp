/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"
#include <telemetry/gleansample.h>
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"
#include <QApplication>

// In seconds, the timeout for unstable pings.
constexpr uint32_t PING_TIME_UNSTABLE_SEC = 2;

// In seconds, the timeout to detect no-signal pings.
constexpr uint32_t PING_TIME_NOSIGNAL_SEC = 4;

// Packet loss threshold for a connection to be considered unstable.
constexpr double PING_LOSS_UNSTABLE_THRESHOLD = 0.10;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionHealth");
}

ConnectionHealth::ConnectionHealth() {
  MVPN_COUNT_CTOR(ConnectionHealth);

  m_noSignalTimer.setSingleShot(true);

  connect(&m_healthCheckTimer, &QTimer::timeout, this,
          &ConnectionHealth::healthCheckup);

  connect(&m_pingHelper, &PingHelper::pingSentAndReceived, this,
          &ConnectionHealth::pingSentAndReceived);

  connect(qApp, &QApplication::applicationStateChanged, this,
          &ConnectionHealth::applicationStateChanged);
}

ConnectionHealth::~ConnectionHealth() { MVPN_COUNT_DTOR(ConnectionHealth); }

void ConnectionHealth::stop() {
  logger.debug() << "ConnectionHealth deactivated";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  setStability(Stable);
}

void ConnectionHealth::start(const QString& serverIpv4Gateway,
                             const QString& deviceIpv4Address) {
  logger.debug() << "ConnectionHealth activated";

  if (m_suspended || serverIpv4Gateway.isEmpty() ||
      MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    return;
  }

  m_currentGateway = serverIpv4Gateway;
  m_deviceAddress = deviceIpv4Address;
  m_pingHelper.start(serverIpv4Gateway, deviceIpv4Address);
  m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE_SEC * 1000);
}

void ConnectionHealth::setStability(ConnectionStability stability) {
  if (m_stability == stability) {
    return;
  }

  logger.debug() << "Stability changed:" << stability;

  if (stability == Unstable) {
    MozillaVPN::instance()->silentSwitch();

    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::connectionHealthUnstable);
  } else if (stability == NoSignal) {
    emit MozillaVPN::instance()->recordGleanEvent(
        GleanSample::connectionHealthNoSignal);
  }

  m_stability = stability;
  emit stabilityChanged();
}

void ConnectionHealth::connectionStateChanged() {
  logger.debug() << "Connection state changed";

  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    stop();
    return;
  }

  MozillaVPN::instance()->controller()->getStatus(
      [this](const QString& serverIpv4Gateway, const QString& deviceIpv4Address,
             uint64_t txBytes, uint64_t rxBytes) {
        Q_UNUSED(txBytes);
        Q_UNUSED(rxBytes);

        stop();
        start(serverIpv4Gateway, deviceIpv4Address);
      });
}

void ConnectionHealth::pingSentAndReceived(qint64 msec) {
#ifdef MVPN_DEBUG
  logger.debug() << "Ping answer received in msec:" << msec;
#else
  Q_UNUSED(msec);
#endif

  // If a ping has been received, we have signal. Restart the timers.
  m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE_SEC * 1000);

  healthCheckup();

  emit pingChanged();
}

void ConnectionHealth::healthCheckup() {
  // If the no-signal timer has elapsed, then we probably lost the connection.
  if (!m_noSignalTimer.isActive()) {
    setStability(NoSignal);
  }
  // If there are too many lost pings, then mark the connection as unstable.
  else if (m_pingHelper.loss() > PING_LOSS_UNSTABLE_THRESHOLD) {
    setStability(Unstable);
  }
  // If recent pings took to long, then mark the connection as unstable.
  else if (m_pingHelper.maximum() > (PING_TIME_UNSTABLE_SEC * 1000)) {
    setStability(Unstable);
  }
  // Otherwise, the connection is stable.
  else {
    setStability(Stable);
  }
}

void ConnectionHealth::applicationStateChanged(Qt::ApplicationState state) {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  Q_UNUSED(state);
  // Do not suspend PingsendHelper on Desktop.
  return;
#else
  switch (state) {
    case Qt::ApplicationState::ApplicationActive:
      if (m_suspended) {
        m_suspended = false;

        Q_ASSERT(!m_noSignalTimer.isActive());
        logger.debug() << "Resuming connection check from Suspension";
        start(m_currentGateway, m_deviceAddress);
      }
      break;

    case Qt::ApplicationState::ApplicationSuspended:
    case Qt::ApplicationState::ApplicationInactive:
    case Qt::ApplicationState::ApplicationHidden:
      logger.debug() << "Pausing connection for Suspension";
      m_suspended = true;
      stop();
      break;
  }
#endif
}
