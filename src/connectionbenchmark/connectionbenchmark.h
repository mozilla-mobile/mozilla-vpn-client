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
  Q_PROPERTY(Speed speed READ speed NOTIFY speedChanged);
  Q_PROPERTY(quint64 download READ download NOTIFY downloadChanged);

 public:
  ConnectionBenchmark();
  ~ConnectionBenchmark();

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();

  enum State {
    StateInitial,
    StateDownloadBenchmarking,
    StateDownloadReady,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

  enum Speed {
    SpeedSlow,
    SpeedMedium,
    SpeedFast,
  };
  Q_ENUM(Speed);

  State state() const { return m_state; }
  Speed speed() const { return m_speed; }
  qint64 download() const { return m_download; }

 signals:
  void stateChanged();
  void speedChanged();
  void downloadChanged();

 private:
  State m_state = StateInitial;
  Speed m_speed;
  qint64 m_download;

  ConnectionBenchmarkDownload* m_benchmarkDownload;

  void setState(State state);
  void setSpeed(qint64 m_download);
};

#endif  // CONNECTIONBENCHMARK_H
