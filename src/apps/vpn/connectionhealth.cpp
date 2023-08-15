/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"

#include <QApplication>
#include <QDateTime>
#include <QRandomGenerator>

#include "controller.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"

namespace {
Logger logger("ConnectionHealth");
}

ConnectionHealth::ConnectionHealth() : m_dnsPingSender(QHostAddress()) {
  MZ_COUNT_CTOR(ConnectionHealth);

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
          &ConnectionHealth::dnsPingReceived, Qt::QueuedConnection);

  connect(&m_dnsPingTimer, &QTimer::timeout, this, [this]() {
    m_dnsPingSequence++;
    m_dnsPingTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_dnsPingSender.sendPing(QHostAddress(PING_WELL_KNOWN_ANYCAST_DNS),
                             m_dnsPingSequence);
  });

  m_dnsPingInitialized = false;
  m_dnsPingLatency = PING_TIME_UNSTABLE_SEC * 1000;
}

ConnectionHealth::~ConnectionHealth() { MZ_COUNT_DTOR(ConnectionHealth); }

void ConnectionHealth::stop() {
  logger.debug() << "ConnectionHealth deactivated";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  m_dnsPingSender.stop();
  m_dnsPingTimer.stop();

  setStability(Stable);
}

void ConnectionHealth::startActive(const QString& serverIpv4Gateway,
                                   const QString& deviceIpv4Address) {
  logger.debug() << "ConnectionHealth active started";

  if (serverIpv4Gateway.isEmpty() ||
      MozillaVPN::instance()->connectionManager()->state() !=
          ConnectionManager::StateOn) {
    return;
  }

  m_currentGateway = serverIpv4Gateway;
  m_deviceAddress = deviceIpv4Address;
  m_pingHelper.start(serverIpv4Gateway, deviceIpv4Address);
  m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE_SEC * 1000);

  m_dnsPingSender.stop();
  m_dnsPingTimer.stop();
}

void ConnectionHealth::startIdle() {
  logger.debug() << "ConnectionHealth idle started";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  // Reset the DNS latency measurement.
  m_dnsPingSequence = QRandomGenerator::global()->bounded(UINT16_MAX);
  m_dnsPingInitialized = false;
  m_dnsPingLatency = PING_TIME_UNSTABLE_SEC * 1000;

  m_dnsPingSender.start();
  m_dnsPingTimer.start(PING_INTERVAL_IDLE_SEC * 1000);

  // Send an initial ping right away.
  m_dnsPingTimestamp = QDateTime::currentMSecsSinceEpoch();
  m_dnsPingSender.sendPing(QHostAddress(PING_WELL_KNOWN_ANYCAST_DNS),
                           m_dnsPingSequence);
}

void ConnectionHealth::setStability(ConnectionStability stability) {
  // Check the stability overwritten flag to see if we are attempting to force
  // overwrite the stability through the inspector command.
  if (m_stabilityOverwritten) {
    logger.debug() << "Connection health stability is overwritten through the "
                      "Inspector commandline. Forced stability value: "
                   << stability;
    return;
  }

  if (stability == Unstable) {
    MozillaVPN::instance()->silentSwitch();

    mozilla::glean::sample::connection_health_unstable.record();
  } else if (stability == NoSignal) {
    mozilla::glean::sample::connection_health_no_signal.record();
  } else {
#if defined(MZ_ANDROID) || defined(MZ_IOS)
    // Count successful health checks only on mobile apps, as they
    // only do health checks when foregrounded.
    mozilla::glean::session::connection_health_stable_count.add();
#endif
  }

  if (m_stability == stability) {
    return;
  }

  logger.debug() << "Stability changed:" << stability;

  m_stability = stability;
  emit stabilityChanged();
}

void ConnectionHealth::connectionStateChanged() {
  ConnectionManager::State state =
      MozillaVPN::instance()->connectionManager()->state();
  logger.debug() << "Connection state changed to" << state;

  if (state != ConnectionManager::StateInitializing) {
    startUnsettledPeriod();
  }

  switch (state) {
    case ConnectionManager::StateOn:
      MozillaVPN::instance()->connectionManager()->getStatus(
          [this](const QString& serverIpv4Gateway,
                 const QString& deviceIpv4Address, uint64_t txBytes,
                 uint64_t rxBytes) {
            Q_UNUSED(txBytes);
            Q_UNUSED(rxBytes);

            stop();
            startActive(serverIpv4Gateway, deviceIpv4Address);
          });
      break;

    case ConnectionManager::StateOff:
      startIdle();
      break;

    default:
      stop();
  }
}

void ConnectionHealth::pingSentAndReceived(qint64 msec) {
#ifdef MZ_DEBUG
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
  updateDnsPingLatency(latency);
}

void ConnectionHealth::updateDnsPingLatency(quint64 latency) {
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
  // If recent pings took too long, then mark the connection as unstable.
  else if (m_dnsPingInitialized &&
           m_pingHelper.maximum() >
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
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
  Q_UNUSED(state);
  // Do not suspend PingsendHelper on Desktop.
  return;
#else
  switch (state) {
    case Qt::ApplicationState::ApplicationActive:
      if (!m_suspended) return;

      m_suspended = false;
      logger.debug() << "Resuming connection check from suspension";
      startActive(m_currentGateway, m_deviceAddress);
      break;

    case Qt::ApplicationState::ApplicationSuspended:
    case Qt::ApplicationState::ApplicationInactive:
    case Qt::ApplicationState::ApplicationHidden:
      if (m_suspended) return;

      m_suspended = true;
      logger.debug() << "Pausing connection for suspension";
      stop();
      break;
  }
#endif
}
