/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include "pinghelper.h"

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
  Q_PROPERTY(uint latency READ latency NOTIFY pingChanged)
  Q_PROPERTY(double loss READ loss NOTIFY pingChanged)

 public:
  ConnectionHealth();
  ~ConnectionHealth();

  ConnectionStability stability() const { return m_stability; }

  uint latency() const { return m_pingHelper.latency(); }
  double loss() const { return m_pingHelper.loss(); }

 public slots:
  void connectionStateChanged();
  void applicationStateChanged(Qt::ApplicationState state);

 signals:
  void stabilityChanged();
  void pingChanged();

 private:
  void stop();
  void start(const QString& serverIpv4Gateway,
             const QString& deviceIpv4Address);

  void pingSentAndReceived(qint64 msec);

  void setStability(ConnectionStability stability);

  void healthCheckup();

 private:
  ConnectionStability m_stability = Stable;

  QTimer m_noSignalTimer;
  QTimer m_healthCheckTimer;

  PingHelper m_pingHelper;

  bool m_suspended = false;
  QString m_currentGateway;
  QString m_deviceAddress;
};

#endif  // CONNECTIONHEALTH_H
