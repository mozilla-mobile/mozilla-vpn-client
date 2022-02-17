/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "connectionhealth.h"

#include <QObject>
#include <QString>

class ConnectionBenchmark final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmark);

  Q_PROPERTY(State state READ state NOTIFY stateChanged);
  Q_PROPERTY(QString testValue READ testValue NOTIFY testValueChanged);
  Q_PROPERTY(quint64 pingValue READ pingValue NOTIFY pingValueChanged);
  Q_PROPERTY(uint pingLatency READ pingLatency NOTIFY pingChanged);

 public:
  enum State {
    StateInitial,
    StateTesting,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

 public:
  ConnectionBenchmark();
  ~ConnectionBenchmark();

  State state() const { return m_state; }

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();

  const quint64& pingValue();
  uint pingLatency() const { return m_connectionHealth.latency(); }
  const QString& testValue() const { return m_testValue; }

 signals:
  void stateChanged();
  void pingChanged();
  void pingValueChanged();
  void testValueChanged();

 private:
  quint64 m_pingValue;

  QString m_testValue = "test value";

  ConnectionHealth m_connectionHealth;

  State m_state = StateInitial;

  void setState(State state);
};

#endif  // CONNECTIONBENCHMARK_H
