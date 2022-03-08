/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskping.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "taskscheduler.h"

#include <QTimer>

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskPing");
}

BenchmarkTaskPing::BenchmarkTaskPing() : Task("BenchmarkTaskPing") {
  MVPN_COUNT_CTOR(BenchmarkTaskPing);
}

BenchmarkTaskPing::~BenchmarkTaskPing() { MVPN_COUNT_DTOR(BenchmarkTaskPing); }

void BenchmarkTaskPing::setState(State state) {
  logger.debug() << "Set state" << state;

  m_state = state;
}

void BenchmarkTaskPing::run() {
  logger.debug() << "Run ping benchmark";

  if (m_state == StateCancelled) {
    emit completed();
    return;
  }

  connect(MozillaVPN::instance()->connectionHealth(),
          &ConnectionHealth::pingChanged, this, [&] {
            logger.debug() << "Ping changed";

            m_pingLatencyAcc +=
                MozillaVPN::instance()->connectionHealth()->latency();
            m_numOfPingSamples++;
          });

  setState(StateActive);

  QTimer::singleShot(Constants::BENCHMARK_PING_MAX_DURATION, this,
                     &BenchmarkTaskPing::handleTaskFinished);
}

void BenchmarkTaskPing::stop() {
  logger.debug() << "Stop ping benchmark";

  if (m_state == StateActive) {
    setState(StateInactive);
    emit completed();
  } else {
    setState(StateCancelled);
  }
}

void BenchmarkTaskPing::handleTaskFinished() {
  logger.debug() << "Handle task finished";

  m_pingLatency = (int)(m_pingLatencyAcc / m_numOfPingSamples + 0.5);

  emit finished(m_pingLatency);
  setState(StateInactive);

  emit completed();
}
