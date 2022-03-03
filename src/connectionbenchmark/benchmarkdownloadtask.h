/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKDOWNLOADTASK_H
#define BENCHMARKDOWNLOADTASK_H

#include "networkrequest.h"
#include "task.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

class BenchmarkDownloadTask final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkDownloadTask)

 public:
  BenchmarkDownloadTask();
  ~BenchmarkDownloadTask();

  void run() override;
  void stop();

 private slots:
  void taskCompleted(const QByteArray& data);
  void taskFailed(QNetworkReply::NetworkError error, const QByteArray& data);
  void taskProgressed(qint64 bytesReceived, qint64 bytesTotal);

 signals:
  void finished(quint64 bytesPerSecond);
  void progressed(qint64 bytesReceived);

 private:
  QUrl m_fileUrl = QUrl(
      "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
      "download?nocache=73d775b0-3082-47fb-8816-d6171c023fa2&size=25000000");
  NetworkRequest* m_request = nullptr;
  QElapsedTimer m_elapsedTimer;
};

#endif  // BENCHMARKDOWNLOADTASK_H
