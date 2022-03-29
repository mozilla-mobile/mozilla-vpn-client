/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
#include "benchmarktaskdownload.h"
#include "benchmarktaskping.h"
#include "connectionhealth.h"
#include "controller.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "taskscheduler.h"

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmark");
}

ConnectionBenchmark::ConnectionBenchmark() {
  MVPN_COUNT_CTOR(ConnectionBenchmark);
}

ConnectionBenchmark::~ConnectionBenchmark() {
  MVPN_COUNT_DTOR(ConnectionBenchmark);
}

void ConnectionBenchmark::initialize() {
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

  connect(controller, &Controller::stateChanged, this,
          &ConnectionBenchmark::handleControllerState);
  connect(vpn->connectionHealth(), &ConnectionHealth::stabilityChanged, this,
          &ConnectionBenchmark::handleStabilityChange);
}

void ConnectionBenchmark::setConnectionSpeed() {
  logger.debug() << "Set speed";

  if (m_bitsPerSec >= Constants::BENCHMARK_THRESHOLD_SPEED_FAST) {
    m_speed = SpeedFast;
  } else if (m_bitsPerSec >= Constants::BENCHMARK_THRESHOLD_SPEED_MEDIUM) {
    m_speed = SpeedMedium;
  } else {
    m_speed = SpeedSlow;
  }

  emit speedChanged();
  setState(StateReady);
}

void ConnectionBenchmark::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmark::start() {
  logger.debug() << "Start connection benchmarking";

  Q_ASSERT(m_state != StateRunning);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  Controller* controller = vpn->controller();
  Controller::State controllerState = controller->state();
  Q_ASSERT(controllerState == Controller::StateOn);

  setState(StateRunning);

  if (vpn->connectionHealth()->stability() == ConnectionHealth::NoSignal) {
    handleStabilityChange();
  }

  // Create ping benchmark
  BenchmarkTaskPing* pingTask = new BenchmarkTaskPing();
  connect(pingTask, &BenchmarkTaskPing::finished, this,
          &ConnectionBenchmark::pingBenchmarked);
  connect(pingTask->sentinel(), &BenchmarkTask::destroyed, this,
          [this, pingTask]() { m_benchmarkTasks.removeOne(pingTask); });
  m_benchmarkTasks.append(pingTask);
  TaskScheduler::scheduleTask(pingTask);

  // Create download benchmark
  BenchmarkTaskDownload* downloadTask =
      new BenchmarkTaskDownload(Constants::BENCHMARK_DOWNLOAD_URL);
  connect(downloadTask, &BenchmarkTaskDownload::finished, this,
          &ConnectionBenchmark::downloadBenchmarked);
  connect(downloadTask->sentinel(), &BenchmarkTask::destroyed, this,
          [this, downloadTask]() { m_benchmarkTasks.removeOne(downloadTask); });
  m_benchmarkTasks.append(downloadTask);
  TaskScheduler::scheduleTask(downloadTask);
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

  m_bitsPerSec = 0;
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

  m_bitsPerSec = bitsPerSec;
  emit bitsPerSecChanged();

  setConnectionSpeed();
}

void ConnectionBenchmark::pingBenchmarked(quint64 pingLatency) {
  logger.debug() << "Benchmarked ping" << pingLatency;

  m_pingLatency = pingLatency;
  emit pingLatencyChanged();
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
