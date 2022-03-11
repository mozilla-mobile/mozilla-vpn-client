/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskping.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

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
    connect(
        MozillaVPN::instance()->connectionHealth(),
        &ConnectionHealth::pingReceived, this, [&] {
          logger.debug() << "Ping received";

          emit finished(MozillaVPN::instance()->connectionHealth()->latency());
          emit completed();
        });
  } else if (state == BenchmarkTask::StateInactive) {
    emit finished(MozillaVPN::instance()->connectionHealth()->latency());
    emit completed();
  }
}
