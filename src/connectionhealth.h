/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include "dnspingsender.h"
#include "pinghelper.h"

// The baseline latency measurement averaged using an Exponentially Weighted
// Moving Average (EWMA), this defines the decay rate.
constexpr uint32_t PING_BASELINE_EWMA_DIVISOR = 8;

class ConnectionHealth final : public QObject {
 public:
  enum ConnectionStability {
    Stable,
    Unstable,
    NoSignal,
  };
  Q_ENUM(ConnectionStability)

 private:
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionHealth)

  Q_PROPERTY(ConnectionStability stability READ stability()
                 NOTIFY stabilityChanged)
  Q_PROPERTY(uint latency READ latency NOTIFY pingReceived)
  Q_PROPERTY(double loss READ loss NOTIFY pingReceived)
  Q_PROPERTY(bool unsettled READ isUnsettled NOTIFY unsettledChanged)

 public:
  ConnectionHealth();
  ~ConnectionHealth();

  ConnectionStability stability() const { return m_stability; }

  void overwriteStabilityForInspector(ConnectionStability stability);

  uint latency() const { return m_pingHelper.latency(); }
  double loss() const { return m_pingHelper.loss(); }
  double stddev() const { return m_pingHelper.stddev(); }
  bool isUnsettled() const { return m_settlingTimer.isActive(); };

 public slots:
  void connectionStateChanged();
  void applicationStateChanged(Qt::ApplicationState state);

 signals:
  void stabilityChanged();
  void unsettledChanged();
  void pingReceived();

 private:
  void stop();
  void startActive(const QString& serverIpv4Gateway,
                   const QString& deviceIpv4Address);
  void startIdle();

  void pingSentAndReceived(qint64 msec);
  void dnsPingReceived(quint16 sequence);
  void updateDnsPingLatency(quint64 latency);

  void setStability(ConnectionStability stability);

  void healthCheckup();
  void startUnsettledPeriod();

 private:
  ConnectionStability m_stability = Stable;

  // This flag is used to check if the connection stability has been overwritten
  // by the inspector command.
  bool m_stabilityOverwritten = false;

  QTimer m_settlingTimer;
  QTimer m_noSignalTimer;
  QTimer m_healthCheckTimer;
  QTimer m_metricsTimer;

  PingHelper m_pingHelper;

  DnsPingSender m_dnsPingSender;
  QTimer m_dnsPingTimer;
  quint16 m_dnsPingSequence = 0;
  quint64 m_dnsPingTimestamp = 0;
  quint64 m_dnsPingLatency = 0;
  bool m_dnsPingInitialized = false;

  bool m_suspended = false;
  QString m_currentGateway;
  QString m_deviceAddress;

#ifndef MZ_MOBILE
  qint64 m_metricsTimerId = -1;
#endif

  void startTimingDistributionMetric(ConnectionStability stability);
  void stopTimingDistributionMetric(ConnectionStability stability);
  void recordMetrics(ConnectionStability oldStability,
                     ConnectionStability newStability);

#ifdef UNIT_TEST
  friend class TestConnectionHealth;
#endif
};

#endif  // CONNECTIONHEALTH_H
