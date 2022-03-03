/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKDOWNLOADRESOURCE_H
#define TASKDOWNLOADRESOURCE_H

#include "networkrequest.h"
#include "task.h"

#include <QByteArray>
#include <QObject>
#include <QNetworkReply>
#include <QUrl>

class TaskDownloadResource final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskDownloadResource)

 public:
  TaskDownloadResource(const QUrl& fileUrl);
  ~TaskDownloadResource();

  void run() override;
  void stop();

 private slots:
  void onProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onCompleted(const QByteArray& data);
  void onFailed(QNetworkReply::NetworkError error, const QByteArray& data);

 signals:
  void completed(QByteArray data, bool hasError);
  void progressed(qint64 bytesReceived);

 private:
  QUrl m_fileUrl;
  NetworkRequest* m_request = nullptr;
};

#endif  // TASKDOWNLOADRESOURCE_H
