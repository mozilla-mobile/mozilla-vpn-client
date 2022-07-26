/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "benchmarktaskupload.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "mozillavpn.h"
#include "uploaddatagenerator.h"

namespace {
Logger logger(LOG_MAIN, "BenchmarkTaskUpload");
}

BenchmarkTaskUpload::BenchmarkTaskUpload(const QUrl& url)
    : BenchmarkTask(Constants::BENCHMARK_MAX_DURATION_UPLOAD),
      m_uploadUrl(url) {
  MVPN_COUNT_CTOR(BenchmarkTaskUpload);

  connect(this, &BenchmarkTask::stateChanged, this,
          &BenchmarkTaskUpload::handleState);
}

BenchmarkTaskUpload::~BenchmarkTaskUpload() {
  MVPN_COUNT_DTOR(BenchmarkTaskUpload);
}

void BenchmarkTaskUpload::handleState(BenchmarkTask::State state) {
  logger.debug() << "Handle state" << state;

  if (state == BenchmarkTask::StateActive) {
    UploadDataGenerator* uploadData = new UploadDataGenerator(64);
    uploadData->open(UploadDataGenerator::ReadOnly);

    m_request = NetworkRequest::createForUploadData(this,
                                                    m_uploadUrl.toString(),
                                                    uploadData);

    connect(m_request, &NetworkRequest::uploadProgressed, this,
            &BenchmarkTaskUpload::uploadProgressed);
    connect(m_request, &NetworkRequest::requestFailed, this,
            &BenchmarkTaskUpload::uploadReady);
    connect(m_request, &NetworkRequest::requestCompleted, this,
            [&](const QByteArray& data) {
              uploadReady(QNetworkReply::NoError, data);
            });

    m_elapsedTimer.start();
  } else if (state == BenchmarkTask::StateInactive) {
    m_request->abort();
  }
}

void BenchmarkTaskUpload::uploadProgressed(qint64 bytesSent,
                                           qint64 bytesTotal) {
#ifdef MVPN_DEBUG
  logger.debug() << "Upload progressed:" << bytesSent << "(sent)"
                 << bytesTotal << "(total)";
#else
  Q_UNUSED(bytesTotal);
#endif

  m_totalBytesSent += bytesSent;
}

void BenchmarkTaskUpload::uploadReady(QNetworkReply::NetworkError error,
                                      const QByteArray& data) {
  logger.debug() << "Upload ready" << error;
  Q_UNUSED(data);

  m_request = nullptr;

  quint64 bitsPerSec = 0;
  double msecs = static_cast<double>(m_elapsedTimer.elapsed());

  if (m_totalBytesSent > 0 && msecs > 0) {
    bitsPerSec = static_cast<quint64>(
       static_cast<double>(m_totalBytesSent * 8) / (msecs / 1000.00));
  }

  bool hasUnexpectedError = (error != QNetworkReply::NoError &&
                             error != QNetworkReply::OperationCanceledError &&
                             error != QNetworkReply::TimeoutError) ||
                            bitsPerSec == 0;
  logger.debug() << "Upload completed" << bitsPerSec << "baud";

  emit finished(bitsPerSec, hasUnexpectedError);
  emit completed();
}
