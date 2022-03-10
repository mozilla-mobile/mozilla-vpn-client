/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskdownload.h"
#include "benchmarktaskping.h"
#include "connectionbenchmark.h"
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

  m_benchmarkTasks.clear();
}

void ConnectionBenchmark::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmark::start() {
  logger.debug() << "Start connection benchmarking";

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
  m_pingBenchmarkTask = new BenchmarkTaskPing();
  connect(m_pingBenchmarkTask, &BenchmarkTaskPing::finished, this,
          &ConnectionBenchmark::pingBenchmarked);
  m_benchmarkTasks.append(m_pingBenchmarkTask);
  TaskScheduler::scheduleTask(m_pingBenchmarkTask);

  // Create download benchmark
  m_downloadBenchmarkTask =
      new BenchmarkTaskDownload(Constants::BENCHMARK_DOWNLOAD_URL);
  connect(m_downloadBenchmarkTask, &BenchmarkTaskDownload::finished, this,
          &ConnectionBenchmark::downloadBenchmarked);
  m_benchmarkTasks.append(m_downloadBenchmarkTask);
  TaskScheduler::scheduleTask(m_downloadBenchmarkTask);

  setState(StateRunning);
}

void ConnectionBenchmark::stop() {
  logger.debug() << "Stop benchmark";

  if (m_state == StateRunning || m_state == StateError) {
    Q_ASSERT(!m_benchmarkTasks.isEmpty());

    for (BenchmarkTask* benchmark : m_benchmarkTasks) {
      benchmark->stop();
    }
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
