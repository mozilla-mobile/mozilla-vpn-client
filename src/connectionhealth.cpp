/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"

#include <QApplication>
#include <QDateTime>
#include <QRandomGenerator>
#include <chrono>

#include "constants.h"
#include "controller.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"

namespace {
Logger logger("ConnectionHealth");

using namespace std::chrono_literals;
// In seconds, the time between pings while the VPN is deactivated.
constexpr std::chrono::seconds PING_INTERVAL_IDLE = 15s;

// In seconds, the timeout for unstable pings.
constexpr std::chrono::milliseconds PING_TIME_UNSTABLE = 1s;

// In seconds, the timeout to detect no-signal pings.
constexpr std::chrono::seconds PING_TIME_NOSIGNAL = 4s;

// Duration of time after a connection change when we should be skeptical
// of network reachability problems.
constexpr std::chrono::seconds SETTLING_TIMEOUT = 3s;

// Packet loss threshold for a connection to be considered unstable.
constexpr double PING_LOSS_UNSTABLE_THRESHOLD = 0.10;

// Destination address for latency measurements when the VPN is
// deactivated. This is the doh.mullvad.net DNS server.
constexpr const char* PING_WELL_KNOWN_ANYCAST_DNS = "194.242.2.2";
}  // namespace

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
  m_dnsPingLatency = PING_TIME_UNSTABLE.count();
}

ConnectionHealth::~ConnectionHealth() { MZ_COUNT_DTOR(ConnectionHealth); }

void ConnectionHealth::stop() {
  logger.debug() << "ConnectionHealth deactivated";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  m_dnsPingSender.stop();
  m_dnsPingTimer.stop();

  stopTimingDistributionMetric(m_stability);
  setStability(Stable);
}

void ConnectionHealth::startActive(const QString& serverIpv4Gateway,
                                   const QString& deviceIpv4Address) {
  logger.debug() << "ConnectionHealth active started";

  if (serverIpv4Gateway.isEmpty() ||
      MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    logger.info() << "ConnectionHealth not starting because no connection";
    return;
  }

  m_currentGateway = serverIpv4Gateway;
  m_deviceAddress = deviceIpv4Address;
  m_pingHelper.start(serverIpv4Gateway, deviceIpv4Address);
  m_noSignalTimer.start(PING_TIME_NOSIGNAL);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE);

  m_dnsPingSender.stop();
  m_dnsPingTimer.stop();
  startTimingDistributionMetric(m_stability);
}

void ConnectionHealth::startIdle() {
  logger.debug() << "ConnectionHealth idle started";

  m_pingHelper.stop();
  m_noSignalTimer.stop();
  m_healthCheckTimer.stop();

  // Reset the DNS latency measurement.
  m_dnsPingSequence = QRandomGenerator::global()->bounded(UINT16_MAX);
  m_dnsPingInitialized = false;
  m_dnsPingLatency = PING_TIME_UNSTABLE.count();

  m_dnsPingSender.start();
  m_dnsPingTimer.start(PING_INTERVAL_IDLE);

  // Send an initial ping right away.
  m_dnsPingTimestamp = QDateTime::currentMSecsSinceEpoch();
  m_dnsPingSender.sendPing(QHostAddress(PING_WELL_KNOWN_ANYCAST_DNS),
                           m_dnsPingSequence);

  stopTimingDistributionMetric(m_stability);
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

  // Connection check pings sometimes come between VPN sessions, triggering
  // setStability. Do not record count metrics in these cases.
  Controller::State state = MozillaVPN::instance()->controller()->state();
  if (state == Controller::StateOn || state == Controller::StateSwitching ||
      state == Controller::StateSilentSwitching) {
    recordMetrics(m_stability, stability);
  }

  if (stability == Unstable) {
    MozillaVPN::instance()->silentSwitch();
  }

  if (m_stability == stability) {
    return;
  }

  logger.debug() << "Stability changed:" << stability;

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
#ifdef MZ_DEBUG
  logger.debug() << "Ping answer received in msec:" << msec;
#else
  Q_UNUSED(msec);
#endif

  // If a ping has been received, we have signal. Restart the timers.
  m_noSignalTimer.start(PING_TIME_NOSIGNAL);
  m_healthCheckTimer.start(PING_TIME_UNSTABLE);

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
               (PING_TIME_UNSTABLE.count() + m_dnsPingLatency)) {
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
  m_settlingTimer.start(SETTLING_TIMEOUT);
}

void ConnectionHealth::applicationStateChanged(Qt::ApplicationState state) {
#ifndef MZ_MOBILE
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

void ConnectionHealth::overwriteStabilityForInspector(
    ConnectionStability stability) {
  if (Constants::inProduction()) {
    qFatal(
        "Connection health stability mode can only be overwritten in Dev "
        "mode!");
  }
  m_stabilityOverwritten = true;
  m_stability = stability;
  emit stabilityChanged();
}

void ConnectionHealth::startTimingDistributionMetric(
    ConnectionStability stability) {
#ifndef MZ_MOBILE
  switch (stability) {
    case ConnectionHealth::Unstable:
      m_metricsTimerId =
          mozilla::glean::connection_health::unstable_time.start();
      break;
    case ConnectionHealth::NoSignal:
      m_metricsTimerId =
          mozilla::glean::connection_health::no_signal_time.start();
      break;
    default:
      m_metricsTimerId = mozilla::glean::connection_health::stable_time.start();
  }
#endif
}

void ConnectionHealth::stopTimingDistributionMetric(
    ConnectionStability stability) {
#ifndef MZ_MOBILE
  if (m_metricsTimerId == -1) {
    logger.info() << "No active health timer for state" << stability;
    return;
  }
  switch (stability) {
    case ConnectionHealth::Unstable:
      mozilla::glean::connection_health::unstable_time.stopAndAccumulate(
          m_metricsTimerId);
      break;
    case ConnectionHealth::NoSignal:
      mozilla::glean::connection_health::no_signal_time.stopAndAccumulate(
          m_metricsTimerId);
      break;
    default:
      mozilla::glean::connection_health::stable_time.stopAndAccumulate(
          m_metricsTimerId);
  }
  m_metricsTimerId = -1;  // used as a signal to prevent turning it off twice
                          // when ConnectionHealth moves between idle and stop.
#endif
}

void ConnectionHealth::recordMetrics(ConnectionStability oldStability,
                                     ConnectionStability newStability) {
  switch (newStability) {
    case ConnectionHealth::Unstable:
      mozilla::glean::connection_health::unstable_count.add();
      break;
    case ConnectionHealth::NoSignal:
      mozilla::glean::connection_health::no_signal_count.add();
      break;
    default:
      mozilla::glean::connection_health::stable_count.add();
  }

  if (oldStability == newStability) {
    logger.debug() << "No stability change for telemetry.";
    return;
  }

  logger.info() << "Recording telemetry for stability change from"
                << oldStability << "to" << newStability;

  stopTimingDistributionMetric(oldStability);
  startTimingDistributionMetric(newStability);

  switch (newStability) {
    case ConnectionHealth::Unstable:
      mozilla::glean::connection_health::changed_to_unstable.record();
      break;
    case ConnectionHealth::NoSignal:
      mozilla::glean::connection_health::changed_to_no_signal.record();
      break;
    default:
      mozilla::glean::connection_health::changed_to_stable.record();
  }
}
