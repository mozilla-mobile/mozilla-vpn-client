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

 public:
  ConnectionHealth();
  ~ConnectionHealth();

  ConnectionStability stability() const { return m_stability; }

 public slots:
  void connectionStateChanged();
  void applicationStateChanged(Qt::ApplicationState state);

 signals:
  void stabilityChanged();

 private:
  void stop();
  void start(const QString& serverIpv4Gateway,
             const QString& deviceIpv4Address);

  void pingSentAndReceived(qint64 msec);

  void setStability(ConnectionStability stability);

  void noSignalDetected();

 private:
  ConnectionStability m_stability = Stable;

  QTimer m_noSignalTimer;

  PingHelper m_pingHelper;

  bool m_suspended = false;
  QString m_currentGateway;
  QString m_deviceAddress;
};

#endif  // CONNECTIONHEALTH_H
