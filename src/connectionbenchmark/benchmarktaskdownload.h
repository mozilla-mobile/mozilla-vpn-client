/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKDOWNLOAD_H
#define BENCHMARKTASKDOWNLOAD_H

#include "benchmarktask.h"

#include <QDnsLookup>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QUrl>

class NetworkRequest;

class BenchmarkTaskDownload final : public BenchmarkTask {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskDownload)

 public:
  explicit BenchmarkTaskDownload(const QUrl& url);
  ~BenchmarkTaskDownload();

 signals:
  void finished(quint64 bytesPerSecond, bool hasUnexpectedError);

 private:
  void connectNetworkRequest(NetworkRequest* request);
  void dnsLookupFinished();
  void downloadProgressed(qint64 bytesReceived, qint64 bytesTotal,
                          QNetworkReply* reply);
  void downloadReady(QNetworkReply::NetworkError error, const QByteArray& data);
  void handleState(BenchmarkTask::State state);

 private:
  QDnsLookup m_dnsLookup;
  QList<NetworkRequest*> m_requests;
  const QUrl m_fileUrl;

  qint64 m_bytesReceived = 0;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKTASKDOWNLOAD_H
