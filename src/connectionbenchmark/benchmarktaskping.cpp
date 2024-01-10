/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskping.h"

#include "connectionhealth.h"
#include "context/constants.h"
#include "leakdetector.h"
#include "logging/logger.h"
#include "mozillavpn.h"

namespace {
Logger logger("BenchmarkTaskPing");
}

BenchmarkTaskPing::BenchmarkTaskPing()
    : BenchmarkTask("BenchmarkTaskPing",
                    Constants::BENCHMARK_MAX_DURATION_PING) {
  MZ_COUNT_CTOR(BenchmarkTaskPing);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskPing::handleState);
}

BenchmarkTaskPing::~BenchmarkTaskPing() { MZ_COUNT_DTOR(BenchmarkTaskPing); }

void BenchmarkTaskPing::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
    connect(MozillaVPN::instance()->connectionHealth(),
            &ConnectionHealth::pingReceived, this, [&] {
              logger.debug() << "Ping received";
              pingReady();
            });
  } else if (state == BenchmarkTask::StateInactive) {
    pingReady();
  }
}

void BenchmarkTaskPing::pingReady() {
  emit finished(MozillaVPN::instance()->connectionHealth()->latency());
  emit completed();
}
