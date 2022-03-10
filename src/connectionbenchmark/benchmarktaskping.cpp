/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktask.h"
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

BenchmarkTaskPing::BenchmarkTaskPing()
    : BenchmarkTask(Constants::BENCHMARK_MAX_DURATION_PING) {
  MVPN_COUNT_CTOR(BenchmarkTaskPing);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskPing::handleState);
}

BenchmarkTaskPing::~BenchmarkTaskPing() { MVPN_COUNT_DTOR(BenchmarkTaskPing); }

void BenchmarkTaskPing::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
    connect(MozillaVPN::instance()->connectionHealth(),
            &ConnectionHealth::pingChanged, this, [&] {
              logger.debug() << "Ping changed";

              m_pingLatencyAcc +=
                  MozillaVPN::instance()->connectionHealth()->latency();
              m_numOfPingSamples++;
            });
  } else if (state == BenchmarkTask::StateInactive) {
    quint64 m_pingLatency = (int)(m_pingLatencyAcc / m_numOfPingSamples + 0.5);
    emit finished(m_pingLatency);
    emit completed();
  }
}
