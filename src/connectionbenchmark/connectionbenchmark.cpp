/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
#include "filedownloader.h"
#include "leakdetector.h"
#include "logger.h"
#include "pinghelper.h"
#include "timersingleshot.h"

#include <QDateTime>
#include <QUrl>
#include <math.h>

namespace {
Logger logger(LOG_NETWORKING, "ConnectionBenchmark");
}

ConnectionBenchmark::ConnectionBenchmark() {
  MVPN_COUNT_CTOR(ConnectionBenchmark);
}

ConnectionBenchmark::~ConnectionBenchmark() {
  MVPN_COUNT_DTOR(ConnectionBenchmark);
}

void ConnectionBenchmark::setState(State state) {
  logger.debug() << "set state" << state;
  m_state = state;
  emit stateChanged();
}

void ConnectionBenchmark::onDownloaded() {
  logger.debug() << "on downloaded" << m_fileDownloader->bytesReceived();

  m_endTime = QDateTime::currentMSecsSinceEpoch();
  quint64 benchmarkRuntime = m_endTime - m_startTime;
  float bytesPerSecond =
      m_fileDownloader->bytesReceived() / benchmarkRuntime * 1000;
  float bitsPerSecond = bytesPerSecond * 8;
  float mBitsPerSecond = bitsPerSecond / pow(1024, 2);

  logger.debug() << "run time" << benchmarkRuntime;
  logger.debug() << "download speed" << mBitsPerSecond;

  m_mBitsPerSecond = mBitsPerSecond;
  emit downloadSpeedChanged();

  if (m_state == StateTesting) {
    setState(StateInitial);
  }
}

void ConnectionBenchmark::start() {
  logger.debug() << "start benchmark";

  m_startTime = QDateTime::currentMSecsSinceEpoch();
  setState(StateTesting);

  QUrl fileUrl(
      "https://warp.cronon.net:8080/"
      "download?nocache=2e556265-c07c-4413-95f9-bd3e450dcc99&size=25000000&"
      "guid=7344510e-c2dc-4478-af50-167a26ccf667");
  m_fileDownloader = new FileDownloader(fileUrl, this);

  connect(m_fileDownloader, &FileDownloader::downloaded, this,
          &ConnectionBenchmark::onDownloaded);
  connect(m_fileDownloader, &FileDownloader::aborted, this,
          &ConnectionBenchmark::onDownloaded);

  // Stop speedtest when max runtime is reached
  TimerSingleShot::create(this, m_maxRunTime, [this]() {
    logger.error() << "max time reached";

    if (m_state == StateTesting) {
      stop();
    }
  });
}

void ConnectionBenchmark::stop() {
  logger.debug() << "stop benchmark";

  m_fileDownloader->abort();
}

const quint64& ConnectionBenchmark::pingValue() {
  logger.debug() << "ping value";

  m_pingValue = 1234;

  return m_pingValue;
}
