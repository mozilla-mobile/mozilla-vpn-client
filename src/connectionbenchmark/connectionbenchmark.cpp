/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkdownload.h"
#include "connectionbenchmark.h"
#include "leakdetector.h"
#include "logger.h"

#include <QList>

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

void ConnectionBenchmark::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmark::setSpeed(qint64 m_download) {
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
}

void ConnectionBenchmark::start() {
  logger.debug() << "Start connection benchmarking";

  m_benchmarkDownload = new ConnectionBenchmarkDownload();

  connect(m_benchmarkDownload, &ConnectionBenchmarkDownload::stateChanged, this,
          [&] {
            logger.debug() << "State changed" << m_benchmarkDownload->state();

            if (m_benchmarkDownload->state() ==
                ConnectionBenchmarkDownload::StateReady) {
              m_download = m_benchmarkDownload->downloadSpeed();
              downloadChanged();

              setSpeed(m_download);
              setState(StateReady);
            }
          });

  m_benchmarkDownload->start();
  setState(StateDownloadBenchmarking);
}

void ConnectionBenchmark::stop() {
  logger.debug() << "Stop benchmark";
  Q_ASSERT(m_benchmarkDownload);

  m_benchmarkDownload->stop();
  setState(StateInitial);
}

