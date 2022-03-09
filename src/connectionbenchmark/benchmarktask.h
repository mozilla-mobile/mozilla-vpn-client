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
  explicit BenchmarkTask();
  ~BenchmarkTask();

  void run() override;
  void stop();

  enum State { StateActive, StateInactive, StateCancelled };

  qint64 executionTime() const { return m_executionTime; }

 signals:
  void stateChanged(State state);

 protected:
  virtual void runInternal() = 0;

 private:
  void setState(State state);

 private:
  State m_state = StateInactive;
  QElapsedTimer m_elapsedTimer;
  qint64 m_executionTime = 0;
};

#endif  // BENCHMARKTASK_H
