/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskdownload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "taskscheduler.h"

#include <QByteArray>

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskDownload");
}

BenchmarkTaskDownload::BenchmarkTaskDownload(const QString& fileUrl)
    : Task("BenchmarkTaskDownload"), m_fileUrl(fileUrl) {
  MVPN_COUNT_CTOR(BenchmarkTaskDownload);
}

BenchmarkTaskDownload::~BenchmarkTaskDownload() {
  MVPN_COUNT_DTOR(BenchmarkTaskDownload);
}

void BenchmarkTaskDownload::setState(State state) {
  logger.debug() << "Set state" << state;

  m_state = state;
}

void BenchmarkTaskDownload::run() {
  logger.debug() << "Run download benchmark";

  if (m_state == StateCancelled) {
    emit completed();
    return;
  }

  setState(StateActive);
  m_request = NetworkRequest::createForGetUrl(this, m_fileUrl);

  connect(m_request, &NetworkRequest::requestCompleted, this,
          [&](const QByteArray& data) {
            handleTaskFinished(QNetworkReply::NoError, data);
          });
  connect(m_request, &NetworkRequest::requestUpdated, this,
          &BenchmarkTaskDownload::taskProgressed);
  connect(m_request, &NetworkRequest::requestFailed, this,
          &BenchmarkTaskDownload::handleTaskFinished);

  m_elapsedTimer.start();

  QTimer::singleShot(Constants::BENCHMARK_DOWNLOAD_MAX_DURATION, this,
                     &BenchmarkTaskDownload::stop);
}

void BenchmarkTaskDownload::stop() {
  logger.debug() << "Stop download benchmark";

  if (m_state == StateActive) {
    Q_ASSERT(m_request);
    m_request->abort();
  } else {
    setState(StateCancelled);
  }
}

void BenchmarkTaskDownload::handleTaskFinished(
    QNetworkReply::NetworkError error, const QByteArray& data) {
  logger.debug() << "Handle task finished" << error;
  Q_UNUSED(data);

  bool hasUnexpectedError = error != QNetworkReply::NoError &&
                            error != QNetworkReply::OperationCanceledError &&
                            error != QNetworkReply::TimeoutError;

  emit finished(m_bytesPerSecond, hasUnexpectedError);
  setState(StateInactive);

  emit completed();
}

void BenchmarkTaskDownload::taskProgressed(qint64 bytesReceived,
                                           qint64 bytesTotal) {
  logger.debug() << "Download progressed";
  Q_UNUSED(bytesTotal);

  m_bytesPerSecond = bytesReceived / m_elapsedTimer.elapsed() * 1000;

  emit progressed(m_bytesPerSecond);
}
