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
#include <QDateTime>
#include <QRandomGenerator>

// In seconds, the time between pings while the VPN is deactivated.
constexpr uint32_t PING_INTERVAL_IDLE_SEC = 15;

// In seconds, the timeout for unstable pings.
constexpr uint32_t PING_TIME_UNSTABLE_SEC = 1;

// In seconds, the timeout to detect no-signal pings.
constexpr uint32_t PING_TIME_NOSIGNAL_SEC = 4;

// Packet loss threshold for a connection to be considered unstable.
constexpr double PING_LOSS_UNSTABLE_THRESHOLD = 0.10;

// Destination address for latency measurements when the VPN is
// deactivated. This is the Google anycast DNS server.
constexpr const char* PING_WELL_KNOWN_ANYCAST_DNS = "8.8.8.8";

// The baseline latency measurement averaged using an Exponentially Weighted
// Moving Average (EWMA), this defines the decay rate.
constexpr uint32_t PING_BASELINE_EWMA_DIVISOR = 8;

// Duration of time after a connection change when we should be skeptical
// of network reachability problems.
constexpr auto SETTLING_TIMEOUT_SEC = 3;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionHealth");
}

ConnectionHealth::ConnectionHealth() : m_dnsPingSender(QHostAddress()) {
  MVPN_COUNT_CTOR(ConnectionHealth);

  m_noSignalTimer.setSingleShot(true);

  m_settlingTimer.setSingleShot(true);
  connect(&m_settlingTimer, &QTimer::timeout, this, [this]() {
    logger.debug() << "Unsettled period over.";
    emit unsettledChanged();
  });

  connect(&m_healthCheckTimer, &QTimer::timeout, this,
          &ConnectionHealth::healthCheckup);

  connect(&m_pingHelper, &PingHelper::pingSentAndReceived, this,
          &ConnectionHealth::pingSentAndReceived);

  connect(qApp, &QApplication::applicationStateChanged, this,
          &ConnectionHealth::applicationStateChanged);

  connect(&m_dnsPingSender, &DnsPingSender::recvPing, this,
          &ConnectionHealth::dnsPingReceived);

  connect(&m_dnsPingTimer, &QTimer::timeout, this, [this]() {
    m_dnsPingSequence++;
    m_dnsPingTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_dnsPingSender.sendPing(QHostAddress(PING_WELL_KNOWN_ANYCAST_DNS),
                             m_dnsPingSequence);
  });

  m_dnsPingInitialized = false;
  m_dnsPingLatency = PING_TIME_UNSTABLE_SEC * 1000;
}

ConnectionHealth::~ConnectionHealth() { MVPN_COUNT_DTOR(ConnectionHealth); }

void ConnectionHealth::stop() {
  logger.debug() << "ConnectionHealth deactivated";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();
  m_dnsPingTimer.stop();

  setStability(Stable);
}

void ConnectionHealth::startActive(const QString& serverIpv4Gateway,
                                   const QString& deviceIpv4Address) {
  logger.debug() << "ConnectionHealth started";

  if (m_suspended || serverIpv4Gateway.isEmpty() ||
      MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    return;
  }

  m_currentGateway = serverIpv4Gateway;
  m_deviceAddress = deviceIpv4Address;
  m_pingHelper.start(serverIpv4Gateway, deviceIpv4Address);
  m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE_SEC * 1000);
  m_dnsPingTimer.stop();
}

void ConnectionHealth::startIdle() {
  logger.debug() << "ConnectionHealth started";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  // Reset the DNS latency measurement.
  m_dnsPingSequence = QRandomGenerator::global()->bounded(UINT16_MAX);
  m_dnsPingInitialized = false;
  m_dnsPingLatency = PING_TIME_UNSTABLE_SEC * 1000;
  m_dnsPingTimer.start(PING_INTERVAL_IDLE_SEC * 1000);

  // Send an initial ping right away.
  m_dnsPingTimestamp = QDateTime::currentMSecsSinceEpoch();
  m_dnsPingSender.sendPing(QHostAddress(PING_WELL_KNOWN_ANYCAST_DNS),
                           m_dnsPingSequence);
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
  Controller::State state = MozillaVPN::instance()->controller()->state();
  logger.debug() << "Connection state changed to" << state;

  if (state != Controller::StateInitializing) {
    startUnsettledPeriod();
  }

  switch (state) {
    case Controller::StateOn:
      MozillaVPN::instance()->controller()->getStatus(
          [this](const QString& serverIpv4Gateway,
                 const QString& deviceIpv4Address, uint64_t txBytes,
                 uint64_t rxBytes) {
            Q_UNUSED(txBytes);
            Q_UNUSED(rxBytes);

            stop();
            startActive(serverIpv4Gateway, deviceIpv4Address);
          });
      break;

    case Controller::StateOff:
      startIdle();
      break;

    default:
      stop();
  }
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
  emit pingReceived();
}

void ConnectionHealth::dnsPingReceived(quint16 sequence) {
  if (sequence != m_dnsPingSequence) {
    return;
  }
  quint64 latency = QDateTime::currentMSecsSinceEpoch() - m_dnsPingTimestamp;
  logger.debug() << "Received DNS ping:" << latency << "msec";

  if (m_dnsPingInitialized) {
    m_dnsPingLatency *= (PING_BASELINE_EWMA_DIVISOR - 1);
    m_dnsPingLatency += latency;
    m_dnsPingLatency /= PING_BASELINE_EWMA_DIVISOR;
  } else {
    m_dnsPingLatency = latency;
    m_dnsPingInitialized = true;
  }
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
  else if (m_pingHelper.maximum() >
           (PING_TIME_UNSTABLE_SEC * 1000 + m_dnsPingLatency)) {
    setStability(Unstable);
  }
  // Otherwise, the connection is stable.
  else {
    setStability(Stable);
  }
}

void ConnectionHealth::startUnsettledPeriod() {
  logger.debug() << "Starting unsettled period.";
  emit unsettledChanged();
  m_settlingTimer.start(SETTLING_TIMEOUT_SEC * 1000);
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
        startActive(m_currentGateway, m_deviceAddress);
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
