/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "benchmarkdownloadtask.h"

class ConnectionBenchmark final : public QObject {
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
    StateRunning,
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
  quint64 download() const { return m_download; }

 signals:
  void stateChanged();
  void speedChanged();
  void downloadChanged();

 private slots:
  void benchmarkedDownload(quint64 bytesPerSecond, bool hasUnexpectedError);

 private:
  void setConnectionSpeed(quint64 m_download);
  void setState(State state);

 private:
  BenchmarkDownloadTask* m_downloadBenchmarkTask = nullptr;

  State m_state = StateInitial;
  Speed m_speed = SpeedSlow;
  quint64 m_download = 0;
};

#endif  // CONNECTIONBENCHMARK_H
