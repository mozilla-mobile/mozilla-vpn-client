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

void ConnectionBenchmark::setConnectionSpeed() {
  logger.debug() << "Set speed";

  if (m_download >= Constants::BENCHMARK_THRESHOLD_SPEED_FAST) {
    m_speed = SpeedFast;
  } else if (m_download >= Constants::BENCHMARK_THRESHOLD_SPEED_MEDIUM &&
             m_download < Constants::BENCHMARK_THRESHOLD_SPEED_FAST) {
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

  if (vpn->connectionHealth()->stability() == ConnectionHealth::NoSignal) {
    handleStabilityChange();
    return;
  }

  connect(controller, &Controller::stateChanged, this,
          &ConnectionBenchmark::stop);
  connect(vpn->connectionHealth(), &ConnectionHealth::stabilityChanged, this,
          &ConnectionBenchmark::handleStabilityChange);

  // Create ping benchmark
  BenchmarkTaskPing* pingTask = new BenchmarkTaskPing();
  connect(pingTask, &BenchmarkTaskPing::finished, this,
          &ConnectionBenchmark::pingBenchmarked);
  connect(pingTask, &Task::completed, this,
          [this, pingTask]() { m_benchmarkTasks.removeOne(pingTask); });
  m_benchmarkTasks.append(pingTask);
  TaskScheduler::scheduleTask(pingTask);

  // Create download benchmark
  BenchmarkTaskDownload* downloadTask =
      new BenchmarkTaskDownload(Constants::BENCHMARK_DOWNLOAD_URL);
  connect(downloadTask, &BenchmarkTaskDownload::finished, this,
          &ConnectionBenchmark::handleControllerState);
  connect(downloadTask, &Task::completed, this,
          [this, downloadTask]() { m_benchmarkTasks.removeOne(downloadTask); });
  m_benchmarkTasks.append(downloadTask);
  TaskScheduler::scheduleTask(downloadTask);

  setState(StateRunning);
}

void ConnectionBenchmark::stop() {
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

  m_download = 0;
  m_ping = 0;

  setState(StateInitial);
}

void ConnectionBenchmark::downloadBenchmarked(quint64 bytesPerSecond,
                                              bool hasUnexpectedError) {
  logger.debug() << "Benchmarked download" << bytesPerSecond;

  if (hasUnexpectedError) {
    setState(StateError);
    return;
  }

  m_download = bytesPerSecond;
  emit downloadChanged();

  setConnectionSpeed();
}

void ConnectionBenchmark::pingBenchmarked(quint64 pingLatency) {
  logger.debug() << "Benchmarked ping" << pingLatency;

  m_ping = pingLatency;
  emit pingChanged();
}

void ConnectionBenchmark::handleControllerState() {
  Controller::State controllerState =
      MozillaVPN::instance()->controller()->state();
  logger.debug() << "Handle controller state" << controllerState;

  if (m_state == StateRunning) {
    setState(StateError);
    stop();
  }
}

void ConnectionBenchmark::handleStabilityChange() {
  logger.debug() << "Handle stability change";

  if (MozillaVPN::instance()->connectionHealth()->stability() ==
      ConnectionHealth::NoSignal) {
    setState(StateError);
    stop();
  };
}
