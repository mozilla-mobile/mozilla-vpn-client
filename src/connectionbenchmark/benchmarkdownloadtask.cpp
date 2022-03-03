/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarkdownloadtask.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "taskscheduler.h"
#include "tasks/downloadresource/taskdownloadresource.h"

#include <QByteArray>

namespace {
Logger logger(LOG_MAIN, "BenchmarkDownloadTask");
}

BenchmarkDownloadTask::BenchmarkDownloadTask() : Task("BenchmarkDownloadTask") {
  MVPN_COUNT_CTOR(BenchmarkDownloadTask);
}

BenchmarkDownloadTask::~BenchmarkDownloadTask() {
  MVPN_COUNT_DTOR(BenchmarkDownloadTask);
}

void BenchmarkDownloadTask::run() {
  logger.debug() << "Run download";

  m_request = NetworkRequest::createForGetUrl(this, m_fileUrl.toString());

  connect(m_request, &NetworkRequest::requestCompleted, this,
          &BenchmarkDownloadTask::taskCompleted);
  connect(m_request, &NetworkRequest::requestUpdated, this,
          &BenchmarkDownloadTask::taskProgressed);
  connect(m_request, &NetworkRequest::requestFailed, this,
          &BenchmarkDownloadTask::taskFailed);

  m_elapsedTimer.start();
}

void BenchmarkDownloadTask::stop() {
  logger.debug() << "Stop download";
  Q_ASSERT(m_request);

  m_request->abort();
}

void BenchmarkDownloadTask::taskCompleted(const QByteArray& data) {
  logger.debug() << "Download completed";

  quint64 bytesPerSecond = data.size() / m_elapsedTimer.elapsed() * 1000;
  logger.debug() << "Download speed" << bytesPerSecond;

  emit finished(bytesPerSecond);
  emit completed();
}

void BenchmarkDownloadTask::taskFailed(QNetworkReply::NetworkError error,
                                       const QByteArray& data) {
  logger.debug() << "Download failed";

  bool hasError = error != QNetworkReply::NoError;
  Q_UNUSED(hasError);

  quint64 bytesPerSecond = data.size() / m_elapsedTimer.elapsed() * 1000;

  emit finished(bytesPerSecond);
  emit completed();
}

void BenchmarkDownloadTask::taskProgressed(qint64 bytesReceived,
                                           qint64 bytesTotal) {
  logger.debug() << "Download progressed";

  Q_UNUSED(bytesTotal);

  quint64 bytesPerSecond = bytesReceived / m_elapsedTimer.elapsed() * 1000;

  emit progressed(bytesPerSecond);
}