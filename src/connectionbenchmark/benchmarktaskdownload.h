/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKDOWNLOAD_H
#define BENCHMARKTASKDOWNLOAD_H

#include "benchmarktask.h"
#include "networkrequest.h"

#include <QByteArray>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class BenchmarkTaskDownload : public BenchmarkTask {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskDownload)

 public:
  BenchmarkTaskDownload(const QString& fileUrl);
  ~BenchmarkTaskDownload();

 signals:
  void finished(quint64 bytesPerSecond, bool hasUnexpectedError);

 private:
  void downloadReady(QNetworkReply::NetworkError error, const QByteArray& data);
  void handleState(BenchmarkTask::State state);

 private:
  NetworkRequest* m_request = nullptr;
  const QString m_fileUrl;
};

#endif  // BENCHMARKTASKDOWNLOAD_H
