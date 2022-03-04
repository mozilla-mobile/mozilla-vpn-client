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

  enum State { StateActive, StateInactive, StateAborted };

 signals:
  void aborted();
  void finished(quint64 bytesPerSecond, bool hasUnexpectedError);
  void progressed(qint64 bytesReceived);

 private slots:
  void handleTaskFinished(QNetworkReply::NetworkError error,
                          const QByteArray& data);
  void taskProgressed(qint64 bytesReceived, qint64 bytesTotal);

 private:
  void setState(State state);

 private:
  State m_state = StateInactive;

  NetworkRequest* m_request = nullptr;
  QElapsedTimer m_elapsedTimer;
  QUrl m_fileUrl = QUrl(
      "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
      "download?nocache=73d775b0-3082-47fb-8816-d6171c023fa2&size=25000000");
  quint64 m_bytesPerSecond = 0;
};

#endif  // BENCHMARKDOWNLOADTASK_H
