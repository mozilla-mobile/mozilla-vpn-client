/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "connectionbenchmarkdownload.h"

class ConnectionBenchmark : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmark);

  Q_PROPERTY(State state READ state NOTIFY stateChanged);
  Q_PROPERTY(quint64 download READ download NOTIFY downloadChanged);
  Q_PROPERTY(quint64 ping READ ping NOTIFY pingChanged);

 public:
  ConnectionBenchmark();
  ~ConnectionBenchmark();

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void reset();

  enum State {
    StateInitial,
    StatePingBenchmarking,
    StatePingReady,
    StateDownloadBenchmarking,
    StateDownloadReady,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

  State state() const { return m_state; }
  qint64 download() const { return m_download; }
  qint64 ping() const { return m_ping; }

 signals:
  void stateChanged();
  void downloadChanged();
  void pingChanged();

 private:
  State m_state = StateInitial;
  qint64 m_download;
  qint64 m_ping;

  ConnectionBenchmarkDownload* m_benchmarkDownload;

  void runNextBenchmark();
  void setState(State state);
};

#endif  // CONNECTIONBENCHMARK_H
