/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarkdownloadtask.h"
#include "connectionbenchmark.h"
#include "leakdetector.h"
#include "logger.h"
#include "taskscheduler.h"

// TODO: Decide on thresholds for connection speeds
constexpr uint32_t SPEED_THRESHOLD_FAST = 3125000;    // 25 Megabit
constexpr uint32_t SPEED_THRESHOLD_MEDIUM = 1250000;  // 10 Megabit

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmark");
}

ConnectionBenchmark::ConnectionBenchmark() {
  MVPN_COUNT_CTOR(ConnectionBenchmark);
}

ConnectionBenchmark::~ConnectionBenchmark() {
  MVPN_COUNT_DTOR(ConnectionBenchmark);
}

void ConnectionBenchmark::setConnectionSpeed(quint64 m_download) {
  logger.debug() << "Set speed" << m_download;

  if (m_download >= SPEED_THRESHOLD_FAST) {
    m_speed = SpeedFast;
  } else if (m_download >= SPEED_THRESHOLD_MEDIUM &&
             m_download < SPEED_THRESHOLD_FAST) {
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

  m_downloadBenchmarkTask = new BenchmarkDownloadTask();
  connect(m_downloadBenchmarkTask, &BenchmarkDownloadTask::finished, this,
          &ConnectionBenchmark::benchmarkedDownload);

  TaskScheduler::scheduleTask(m_downloadBenchmarkTask);

  setState(StateRunning);
}

void ConnectionBenchmark::stop() {
  logger.debug() << "Stop benchmark";

  if (m_state == StateRunning) {
    Q_ASSERT(m_downloadBenchmarkTask);

    m_downloadBenchmarkTask->stop();
    m_downloadBenchmarkTask = nullptr;
  };

  setState(StateInitial);
}

void ConnectionBenchmark::benchmarkedDownload(quint64 bytesPerSecond,
                                              bool hasUnexpectedError) {
  logger.debug() << "Benchmarked dowload" << bytesPerSecond;

  if (hasUnexpectedError) {
    setState(StateError);
    return;
  }

  m_download = bytesPerSecond;
  downloadChanged();

  setConnectionSpeed(m_download);
}
