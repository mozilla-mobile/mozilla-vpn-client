/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktask.h"
#include "leakdetector.h"
#include "logger.h"
#include "timersingleshot.h"

namespace {
Logger logger(LOG_MAIN, "BenchmarkTask");
}

BenchmarkTask::BenchmarkTask(uint32_t maxExecutionTime)
    : Task("BenchmarkTask"), m_maxExecutionTime(maxExecutionTime) {
  MVPN_COUNT_CTOR(BenchmarkTask);
}

BenchmarkTask::~BenchmarkTask() { MVPN_COUNT_DTOR(BenchmarkTask); }

void BenchmarkTask::setState(State state) {
  logger.debug() << "Set state" << state;

  m_state = state;
  emit stateChanged(m_state);
}

void BenchmarkTask::run() {
  logger.debug() << "Run benchmark";

  if (m_state == StateCancelled) {
    emit completed();
    return;
  }

  setState(StateActive);

  TimerSingleShot::create(this, m_maxExecutionTime, [this]() { stop(); });
}

void BenchmarkTask::stop() {
  logger.debug() << "Stop benchmark";

  if (m_state == StateActive) {
    setState(StateInactive);
  } else {
    setState(StateCancelled);
  }
}
