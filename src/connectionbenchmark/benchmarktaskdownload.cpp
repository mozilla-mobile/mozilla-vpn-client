/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskdownload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QByteArray>

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskDownload");
}

BenchmarkTaskDownload::BenchmarkTaskDownload(const QString& fileUrl)
    : BenchmarkTask(Constants::BENCHMARK_MAX_DURATION_DOWNLOAD),
      m_fileUrl(fileUrl) {
  MVPN_COUNT_CTOR(BenchmarkTaskDownload);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskDownload::handleState);
}

BenchmarkTaskDownload::~BenchmarkTaskDownload() {
  MVPN_COUNT_DTOR(BenchmarkTaskDownload);
}

void BenchmarkTaskDownload::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
    m_request = NetworkRequest::createForGetUrl(this, m_fileUrl);
    connect(m_request, &NetworkRequest::requestUpdated, this,
            &BenchmarkTaskDownload::downloadProgressed);
    connect(m_request, &NetworkRequest::requestFailed, this,
            &BenchmarkTaskDownload::downloadReady);
    connect(m_request, &NetworkRequest::requestCompleted, this,
            [&](const QByteArray& data) {
              downloadReady(QNetworkReply::NoError, data);
            });
  } else if (state == BenchmarkTask::StateInactive && m_request) {
    m_request->abort();
    m_request = nullptr;
  }
}

void BenchmarkTaskDownload::downloadProgressed(qint64 bytesReceived,
                                               qint64 bytesTotal,
                                               QNetworkReply* reply) {
  logger.debug() << "Handle progressed:" << bytesReceived << "(received)"
                 << bytesTotal << "(total)";

  m_bytesReceived = bytesReceived;

  // Discard downloaded data
  QByteArray data = reply->readAll();
  Q_UNUSED(data);
}

void BenchmarkTaskDownload::downloadReady(QNetworkReply::NetworkError error,
                                          const QByteArray& data) {
  logger.debug() << "Download ready" << error;
  Q_UNUSED(data);

  m_request = nullptr;

  quint64 bitsPerSec = 0;
  double msecs = static_cast<double>(executionTime());
  if (m_bytesReceived > 0 && msecs > 0) {
    bitsPerSec = static_cast<quint64>(static_cast<double>(m_bytesReceived * 8) /
                                      (msecs / 1000.00));
  }

  bool hasUnexpectedError = (error != QNetworkReply::NoError &&
                             error != QNetworkReply::OperationCanceledError &&
                             error != QNetworkReply::TimeoutError) ||
                            bitsPerSec == 0;

  emit finished(bitsPerSec, hasUnexpectedError);
  emit completed();
}
