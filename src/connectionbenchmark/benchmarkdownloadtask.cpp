/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarkdownloadtask.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "taskscheduler.h"

#include <QByteArray>

namespace {
Logger logger(LOG_MAIN, "BenchmarkDownloadTask");
}

BenchmarkDownloadTask::BenchmarkDownloadTask(const QString& fileUrl)
    : Task("BenchmarkDownloadTask"), m_fileUrl(fileUrl) {
  MVPN_COUNT_CTOR(BenchmarkDownloadTask);
}

BenchmarkDownloadTask::~BenchmarkDownloadTask() {
  MVPN_COUNT_DTOR(BenchmarkDownloadTask);
}

void BenchmarkDownloadTask::setState(State state) {
  logger.debug() << "Set state" << state;

  m_state = state;
}

void BenchmarkDownloadTask::run() {
  logger.debug() << "Run download benchmark";

  if (m_state == StateCancelled) {
    emit completed();
  }

  setState(StateActive);
  m_request = NetworkRequest::createForGetUrl(this, m_fileUrl);

  connect(m_request, &NetworkRequest::requestCompleted, this,
          [&](const QByteArray& data) {
            handleTaskFinished(QNetworkReply::NoError, data);
          });
  connect(m_request, &NetworkRequest::requestUpdated, this,
          &BenchmarkDownloadTask::taskProgressed);
  connect(m_request, &NetworkRequest::requestFailed, this,
          &BenchmarkDownloadTask::handleTaskFinished);

  m_elapsedTimer.start();
}

void BenchmarkDownloadTask::stop() {
  logger.debug() << "Stop download benchmark";

  if (m_state == StateActive) {
    Q_ASSERT(m_request);
    m_request->abort();
  } else {
    setState(StateCancelled);
  }
}

void BenchmarkDownloadTask::handleTaskFinished(
    QNetworkReply::NetworkError error, const QByteArray& data) {
  logger.debug() << "Handle task finished" << data.size();
  Q_UNUSED(data);

  bool hasUnexpectedError = error != QNetworkReply::NoError &&
                            error != QNetworkReply::OperationCanceledError;

  emit finished(m_bytesPerSecond, hasUnexpectedError);
  setState(StateInactive);

  emit completed();
}

void BenchmarkDownloadTask::taskProgressed(qint64 bytesReceived,
                                           qint64 bytesTotal) {
  logger.debug() << "Download progressed";
  Q_UNUSED(bytesTotal);

  m_bytesPerSecond = bytesReceived / m_elapsedTimer.elapsed() * 1000;

  emit progressed(m_bytesPerSecond);
}
