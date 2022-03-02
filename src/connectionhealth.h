/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include "pinghelper.h"
#include "dnspingsender.h"

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

 public:
  ConnectionHealth();
  ~ConnectionHealth();

  ConnectionStability stability() const { return m_stability; }

  uint latency() const { return m_pingHelper.latency(); }
  double loss() const { return m_pingHelper.loss(); }
  bool isUnsettled() const { return m_settlingTimer.isActive(); };

 public slots:
  void connectionStateChanged();
  void applicationStateChanged(Qt::ApplicationState state);

 signals:
  void stabilityChanged();
  void pingReceived();

 private:
  void stop();
  void startActive(const QString& serverIpv4Gateway,
                   const QString& deviceIpv4Address);
  void startIdle();

  void pingSentAndReceived(qint64 msec);
  void dnsPingReceived(quint16 sequence);

  void setStability(ConnectionStability stability);

  void healthCheckup();
  void startUnsettledPeriod();

 private:
  ConnectionStability m_stability = Stable;

  QTimer m_settlingTimer;
  QTimer m_noSignalTimer;
  QTimer m_healthCheckTimer;

  PingHelper m_pingHelper;

  DnsPingSender m_dnsPingSender;
  QTimer m_dnsPingTimer;
  quint16 m_dnsPingSequence = 0;
  quint64 m_dnsPingTimestamp = 0;
  quint64 m_dnsPingLatency = 0;

  bool m_suspended = false;
  QString m_currentGateway;
  QString m_deviceAddress;
};

#endif  // CONNECTIONHEALTH_H
