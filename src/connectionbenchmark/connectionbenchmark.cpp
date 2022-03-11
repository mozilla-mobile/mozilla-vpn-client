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

  m_connectionHealth = vpn->connectionHealth();
  if (m_connectionHealth->stability() == ConnectionHealth::NoSignal) {
    handleStabilityChange();
  }

  connect(controller, &Controller::stateChanged, this,
          &ConnectionBenchmark::stop);
  connect(m_connectionHealth, &ConnectionHealth::stabilityChanged, this,
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
          &ConnectionBenchmark::downloadBenchmarked);
  connect(downloadTask, &Task::completed, this,
          [this, downloadTask]() { m_benchmarkTasks.removeOne(downloadTask); });
  m_benchmarkTasks.append(downloadTask);
  TaskScheduler::scheduleTask(downloadTask);

  setState(StateRunning);
}

void ConnectionBenchmark::stop() {
  logger.debug() << "Stop benchmark";

  if (m_state == StateRunning || m_state == StateError) {
    Q_ASSERT(!m_benchmarkTasks.isEmpty());

    for (BenchmarkTask* benchmark : m_benchmarkTasks) {
      benchmark->stop();
    }

    m_benchmarkTasks.clear();
  };

  if (m_state != StateError) {
    setState(StateInitial);
  }
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

void ConnectionBenchmark::handleStabilityChange() {
  logger.debug() << "Handle stability change";

  if (m_connectionHealth->stability() == ConnectionHealth::NoSignal) {
    setState(StateError);
    stop();
  };
}
