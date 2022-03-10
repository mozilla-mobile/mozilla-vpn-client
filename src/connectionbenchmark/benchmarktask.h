/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASK_H
#define BENCHMARKTASK_H

#include "task.h"

#include <QElapsedTimer>
#include <QObject>

class BenchmarkTask : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTask)

 public:
  BenchmarkTask(const uint32_t& maxExecutionTime);
  ~BenchmarkTask();

  void run() override;
  void stop();

  enum State { StateActive, StateInactive, StateCancelled };

  State state() const { return m_state; }
  qint64 executionTime() const { return m_executionTime; }

 signals:
  void stateChanged(State state);

 protected:
  virtual void runInternal() = 0;

 private:
  void setState(State state);

 private:
  State m_state = StateInactive;
  qint64 m_executionTime = 0;

  const uint32_t m_maxExecutionTime;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKTASK_H
