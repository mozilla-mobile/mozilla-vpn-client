/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKDOWNLOAD_H
#define BENCHMARKTASKDOWNLOAD_H

#include "networkrequest.h"
#include "task.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class BenchmarkTaskDownload final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskDownload)

 public:
  explicit BenchmarkTaskDownload(const QString& fileUrl);
  ~BenchmarkTaskDownload();

  void run() override;
  void stop();

  enum State { StateActive, StateInactive, StateCancelled };

 signals:
  void aborted();
  void finished(quint64 bytesPerSecond, bool hasUnexpectedError);
  void progressed(qint64 bytesReceived);

 private:
  void handleTaskFinished(QNetworkReply::NetworkError error,
                          const QByteArray& data);
  void taskProgressed(qint64 bytesReceived, qint64 bytesTotal);
  void setState(State state);

 private:
  State m_state = StateInactive;

  NetworkRequest* m_request = nullptr;
  quint64 m_bytesPerSecond = 0;

  QElapsedTimer m_elapsedTimer;
  const QString m_fileUrl;
};

#endif  // BENCHMARKTASKDOWNLOAD_H
