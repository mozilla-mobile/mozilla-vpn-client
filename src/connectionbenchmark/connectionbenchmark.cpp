/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"

#include "constants.h"
#include "benchmarktaskping.h"
#include "benchmarktasktransfer.h"
#include "connectionhealth.h"
#include "controller.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "taskscheduler.h"

namespace {
Logger logger("ConnectionBenchmark");
}

ConnectionBenchmark::ConnectionBenchmark() {
  MZ_COUNT_CTOR(ConnectionBenchmark);
}

ConnectionBenchmark::~ConnectionBenchmark() {
  MZ_COUNT_DTOR(ConnectionBenchmark);
}

void ConnectionBenchmark::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

  connect(controller, &Controller::stateChanged, this,
          &ConnectionBenchmark::handleControllerState);
  connect(vpn->connectionHealth(), &ConnectionHealth::stabilityChanged, this,
          &ConnectionBenchmark::handleStabilityChange);
}

void ConnectionBenchmark::setConnectionSpeed() {
  if (m_state == StateError) {
    logger.debug() << "Set connection speed abort because of errors";
    return;
  }

  logger.debug() << "Set connection speed";

  // TODO: Take uploadBps for calculating speed into account
  if (m_downloadBps >= Constants::BENCHMARK_THRESHOLD_SPEED_FAST) {
    m_speed = SpeedFast;
  } else if (m_downloadBps >= Constants::BENCHMARK_THRESHOLD_SPEED_MEDIUM) {
    m_speed = SpeedMedium;
  } else {
    m_speed = SpeedSlow;
  }

  mozilla::glean::sample::speed_test_completed.record(
      mozilla::glean::sample::SpeedTestCompletedExtra{
          ._speed = QVariant::fromValue(m_speed).toString()});

  emit speedChanged();
  setState(StateReady);
}

void ConnectionBenchmark::setState(State state) {
  // Wrapped in if statement so only log this once when have both upload and
  // download errors
  if (state == StateError && m_state != StateError) {
    mozilla::glean::sample::speed_test_completed.record(
        mozilla::glean::sample::SpeedTestCompletedExtra{._speed = "Error"});
  }
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmark::start() {
  logger.debug() << "Start connection benchmarking";

  Q_ASSERT(m_state != StateRunning);

  MozillaVPN* vpn = MozillaVPN::instance();

  Controller* controller = vpn->controller();
  Controller::State controllerState = controller->state();
  Q_ASSERT(controllerState == Controller::StateOn);

  setState(StateRunning);

  // Create ping benchmark
  BenchmarkTaskPing* pingTask = new BenchmarkTaskPing();
  connect(pingTask, &BenchmarkTaskPing::finished, this,
          &ConnectionBenchmark::pingBenchmarked);
  connect(pingTask->sentinel(), &BenchmarkTaskSentinel::sentinelDestroyed, this,
          [this, pingTask]() { m_benchmarkTasks.removeOne(pingTask); });
  m_benchmarkTasks.append(pingTask);
  TaskScheduler::scheduleTask(pingTask);

  // Create download benchmark
  BenchmarkTaskTransfer* downloadTask = new BenchmarkTaskTransfer(
      "BenchmarkTaskDownload", BenchmarkTaskTransfer::BenchmarkDownload,
      Constants::benchmarkDownloadUrl());
  connect(downloadTask, &BenchmarkTaskTransfer::finished, this,
          &ConnectionBenchmark::downloadBenchmarked);
  connect(downloadTask->sentinel(), &BenchmarkTaskSentinel::sentinelDestroyed,
          this,
          [this, downloadTask]() { m_benchmarkTasks.removeOne(downloadTask); });
  m_benchmarkTasks.append(downloadTask);
  TaskScheduler::scheduleTask(downloadTask);

  // Create upload benchmark
  if (Feature::get(Feature::Feature_benchmarkUpload)->isSupported()) {
    BenchmarkTaskTransfer* uploadTask = new BenchmarkTaskTransfer(
        "BenchmarkTaskUpload", BenchmarkTaskTransfer::BenchmarkUpload,
        Constants::benchmarkUploadUrl());
    Q_UNUSED(uploadTask);

    connect(uploadTask, &BenchmarkTaskTransfer::finished, this,
            &ConnectionBenchmark::uploadBenchmarked);
    connect(uploadTask->sentinel(), &BenchmarkTask::destroyed, this,
            [this, uploadTask]() { m_benchmarkTasks.removeOne(uploadTask); });
    m_benchmarkTasks.append(uploadTask);
    TaskScheduler::scheduleTask(uploadTask);
  }
}

void ConnectionBenchmark::stop() {
  if (m_state == StateInitial) {
    return;
  }

  logger.debug() << "Stop benchmarks";
  if ((m_state == StateRunning || m_state == StateError) &&
      !m_benchmarkTasks.isEmpty()) {
    for (BenchmarkTask* benchmark : m_benchmarkTasks) {
      benchmark->stop();
    }

    m_benchmarkTasks.clear();
  };
}

void ConnectionBenchmark::reset() {
  logger.debug() << "Reset benchmarks";

  stop();

  m_downloadBps = 0;
  m_uploadBps = 0;
  m_pingLatency = 0;

  setState(StateInitial);
}

void ConnectionBenchmark::downloadBenchmarked(quint64 bitsPerSec,
                                              bool hasUnexpectedError) {
  logger.debug() << "Benchmarked download" << bitsPerSec;

  if (hasUnexpectedError) {
    setState(StateError);
    return;
  }

  m_downloadBps = bitsPerSec;
  emit downloadBpsChanged();

  if (!Feature::get(Feature::Feature_benchmarkUpload)->isSupported()) {
    // All benchmarks ran successfully and we can set the connection speed.
    setConnectionSpeed();
  }
}

void ConnectionBenchmark::pingBenchmarked(quint64 pingLatency) {
  logger.debug() << "Benchmarked ping" << pingLatency;

  m_pingLatency = pingLatency;
  emit pingLatencyChanged();
}

void ConnectionBenchmark::uploadBenchmarked(quint64 bitsPerSec,
                                            bool hasUnexpectedError) {
  logger.debug() << "Benchmarked upload" << bitsPerSec;

  if (hasUnexpectedError) {
    setState(StateError);
    return;
  }

  m_uploadBps = bitsPerSec;
  emit uploadBpsChanged();

  if (Feature::get(Feature::Feature_benchmarkUpload)->isSupported()) {
    // All benchmarks ran successfully and we can set the connection speed.
    setConnectionSpeed();
  }
}

void ConnectionBenchmark::handleControllerState() {
  if (m_state == StateInitial || m_state == StateReady) {
    return;
  }

  Controller::State controllerState =
      MozillaVPN::instance()->controller()->state();
  logger.debug() << "Handle controller state" << controllerState;

  if (controllerState != Controller::StateOn) {
    setState(StateError);
    stop();
  }
}

void ConnectionBenchmark::handleStabilityChange() {
  if (m_state == StateInitial || m_state == StateReady) {
    return;
  }

  ConnectionHealth::ConnectionStability stability =
      MozillaVPN::instance()->connectionHealth()->stability();
  logger.debug() << "Handle stability change" << stability;

  if (stability == ConnectionHealth::NoSignal) {
    setState(StateError);
    stop();
  };
}
