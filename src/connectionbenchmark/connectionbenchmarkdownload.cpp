/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkdownload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "taskscheduler.h"
#include "tasks/downloadresource/taskdownloadresource.h"

namespace {
Logger logger(LOG_NETWORKING, "ConnectionBenchmarkDownload");
}

ConnectionBenchmarkDownload::ConnectionBenchmarkDownload() {
  MVPN_COUNT_CTOR(ConnectionBenchmarkDownload);
}

ConnectionBenchmarkDownload::~ConnectionBenchmarkDownload() {
  MVPN_COUNT_DTOR(ConnectionBenchmarkDownload);
}

void ConnectionBenchmarkDownload::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmarkDownload::start() {
  logger.debug() << "Start benchmark";

  m_bytesPerSecond = 0;
  m_downloadTask = new TaskDownloadResource(QUrl(
      "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
      "download?nocache=73d775b0-3082-47fb-8816-d6171c023fa2&size=25000000"));

  connect(m_downloadTask, &TaskDownloadResource::completed, this,
          &ConnectionBenchmarkDownload::onReady);

  m_timer.start();
  TaskScheduler::scheduleTask(m_downloadTask);
  setState(StateBenchmarking);
}

void ConnectionBenchmarkDownload::stop() {
  logger.debug() << "Stop benchmark";
  Q_ASSERT(m_downloadTask);

  m_downloadTask->stop();
}

void ConnectionBenchmarkDownload::onReady(QByteArray data, bool hasError) {
  logger.debug() << "On download task ready";

  if (hasError) {
    setState(StateError);
    return;
  }

  m_bytesPerSecond = data.size() / m_timer.elapsed() * 1000;

  setState(StateReady);
}
