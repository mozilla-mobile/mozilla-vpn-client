/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include <QList>
#include <QObject>
#include <QThread>
#include <QTimer>

class Server;
class PingSender;

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

  void start(const QString& serverIpv4Gateway);

  void stop();

  ConnectionStability stability() const { return m_stability; }

 public slots:
  void connectionStateChanged();

 signals:
  void stabilityChanged();

 private:
  void setStability(ConnectionStability stability);
  void nextPing();

  void pingReceived(PingSender* pingSender, qint64 msec);
  void noSignalDetected();

 private:
  ConnectionStability m_stability = Stable;

  QString m_gateway;

  QTimer m_pingTimer;
  QTimer m_noSignalTimer;

  QList<PingSender*> m_pings;

  QThread m_pingThread;
};

#endif  // CONNECTIONHEALTH_H
