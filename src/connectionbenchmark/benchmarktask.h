/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASK_H
#define BENCHMARKTASK_H

#include "task.h"

#include <QElapsedTimer>

class BenchmarkTask : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTask)

 public:
  BenchmarkTask(uint32_t maxExecutionTime);
  virtual ~BenchmarkTask();

  void run() override;
  void stop();

  enum State { StateActive, StateInactive, StateCancelled };

  State state() const { return m_state; }
  qint64 executionTime() const { return m_elapsedTimer.elapsed(); }

 signals:
  void stateChanged(State state);

 private:
  void setState(State state);

 private:
  State m_state = StateInactive;

  const uint32_t m_maxExecutionTime;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKTASK_H
