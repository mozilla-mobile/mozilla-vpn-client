/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKTRANSFER_H
#define BENCHMARKTASKTRANSFER_H

#include <QDnsLookup>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QUrl>

#include "benchmarktask.h"

class NetworkRequest;

class BenchmarkTaskTransfer : public BenchmarkTask {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskTransfer)
  Q_PROPERTY(BenchmarkType type MEMBER m_type CONSTANT)

 public:
  enum BenchmarkType {
    BenchmarkDownload,
    BenchmarkUpload,
  };

  explicit BenchmarkTaskTransfer(const QString& name, BenchmarkType type,
                                 const QUrl& url);
  virtual ~BenchmarkTaskTransfer();

 signals:
  void finished(quint64 bitsPerSec, bool hasUnexpectedError);

 private:
  void createNetworkRequest();
  void createNetworkRequestWithRecord(const QDnsHostAddressRecord& record);
  void connectNetworkRequest(NetworkRequest* request);
  void dnsLookupFinished();
  void handleState(BenchmarkTask::State state);
  void transferProgressed(qint64 bytesTransferred, qint64 bytesTotal,
                          QNetworkReply* reply);
  void transferReady(QNetworkReply::NetworkError error, const QByteArray& data);

 private:
  BenchmarkType m_type;
  QDnsLookup m_dnsLookup;
  QList<NetworkRequest*> m_requests;
  const QUrl m_url;

  qint64 m_bytesTransferred = 0;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKTASKTRANSFER_H
