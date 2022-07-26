/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKUPLOAD_H
#define BENCHMARKTASKUPLOAD_H

#include "benchmarktask.h"

#include <QDnsLookup>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QUrl>

class NetworkRequest;

class BenchmarkTaskUpload final : public BenchmarkTask {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskUpload)

 public:
  explicit BenchmarkTaskUpload(const QUrl& url);
  ~BenchmarkTaskUpload();

 signals:
  void finished(quint64 bitsPerSec, bool hasUnexpectedError);

 private:
  void handleState(BenchmarkTask::State state);
  void uploadProgressed(qint64 bytesSent, qint64 bytesTotal);
  void uploadReady(QNetworkReply::NetworkError error, const QByteArray& data);

 private:
  NetworkRequest* m_request = nullptr;
  const QUrl m_uploadUrl;

  qint64 m_totalBytesSent = 0;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKTASKUPLOAD_H
