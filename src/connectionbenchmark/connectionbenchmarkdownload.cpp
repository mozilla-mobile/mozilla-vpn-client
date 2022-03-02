/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkdownload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "taskscheduler.h"
#include "tasks/downloadresource/taskdownloadresource.h"

#include <QDateTime>

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
  m_startTime = QDateTime::currentMSecsSinceEpoch();

  TaskDownloadResource* downloadTask = new TaskDownloadResource(QUrl(
      "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
      "download?nocache=73d775b0-3082-47fb-8816-d6171c023fa2&size=25000000"));

  connect(downloadTask, &TaskDownloadResource::completed, this,
          &ConnectionBenchmarkDownload::onReady);

  TaskScheduler::scheduleTask(downloadTask);
  setState(StateBenchmarking);
}

void ConnectionBenchmarkDownload::onReady(QByteArray data, bool hasError) {
  logger.debug() << "On download task ready";

  if (hasError) {
    setState(StateError);
    return;
  }

  quint64 downloadDuration = QDateTime::currentMSecsSinceEpoch() - m_startTime;
  m_bytesPerSecond = data.size() / downloadDuration * 1000;

  setState(StateReady);
}
