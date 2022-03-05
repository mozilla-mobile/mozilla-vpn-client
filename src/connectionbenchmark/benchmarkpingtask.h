/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKPINGTASK_H
#define BENCHMARKPINGTASK_H

#include "networkrequest.h"
#include "task.h"

#include <QElapsedTimer>
#include <QObject>
#include <QVector>

class BenchmarkPingTask final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkPingTask)

 public:
  BenchmarkPingTask();
  ~BenchmarkPingTask();

  void run() override;
  void stop();

  enum State { StateActive, StateInactive, StateCancelled };

 signals:
  void aborted();
  void finished(quint16 pingLatency, bool hasUnexpectedError);

 private slots:
  void handleTaskFinished();

 private:
  void setState(State state);

 private:
  State m_state = StateInactive;

  double m_pingLatencyAcc = 0;
  quint8 m_numOfPingSamples = 0;
  quint16 m_pingLatency = 0;
};

#endif  // BENCHMARKPINGTASK_H
