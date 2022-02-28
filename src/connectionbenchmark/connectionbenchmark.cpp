/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkdownload.h"
#include "connectionbenchmark.h"
#include "leakdetector.h"
#include "logger.h"

#include <QList>

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

// TODO: Decide on thresholds for connection speeds
void ConnectionBenchmark::setSpeed(qint64 m_ping, qint64 m_download) {
  logger.debug() << "Set speed" << m_ping << m_download;

  if (m_download < 10) {
    m_speed = SpeedSlow;
  } else if (m_download < 25) {
    m_speed = SpeedMedium;
  } else {
    m_speed = SpeedHigh;
  }

  emit speedChanged();
}

void ConnectionBenchmark::runNextBenchmark() {
  if (m_state == StatePingBenchmarking) {
    logger.debug() << "Run ping benchmark";

    m_ping = 15;
    pingChanged();

    setState(StatePingReady);
    runNextBenchmark();
  } else if (m_state == StatePingReady) {
    logger.debug() << "Run download benchmark";
    setState(StateDownloadBenchmarking);

    m_benchmarkDownload = new ConnectionBenchmarkDownload();
    connect(m_benchmarkDownload, &ConnectionBenchmarkDownload::stateChanged,
            this, [&] {
              logger.debug() << "Download speedtest state changed";

              if (m_benchmarkDownload->state() ==
                  ConnectionBenchmarkDownload::StateReady) {
                m_download = m_benchmarkDownload->downloadSpeed();
                downloadChanged();

                setState(StateDownloadReady);
                setSpeed(m_ping, m_download);
                runNextBenchmark();
              } else if (m_benchmarkDownload->state() ==
                         ConnectionBenchmarkDownload::StateError) {
                setState(StateError);
              }
            });

    m_benchmarkDownload->start();
  } else if (m_state == StateDownloadReady) {
    setState(StateReady);
  }
}

void ConnectionBenchmark::start() {
  logger.debug() << "Start connection benchmarking";

  setState(StatePingBenchmarking);
  runNextBenchmark();
}

void ConnectionBenchmark::stop() {
  logger.debug() << "Stop benchmark";

  Q_ASSERT(m_benchmarkDownload);
  m_benchmarkDownload->stop();
}

void ConnectionBenchmark::reset() {
  logger.debug() << "Reset connection benchmarks";

  setState(StateInitial);
}
