/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskdownloadresource.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QUrl>

namespace {
Logger logger(LOG_MAIN, "TaskDownloadResource");
}

TaskDownloadResource::TaskDownloadResource(const QUrl& fileUrl)
    : Task("TaskDownloadResource"), m_fileUrl(fileUrl) {
  MVPN_COUNT_CTOR(TaskDownloadResource);
}

TaskDownloadResource::~TaskDownloadResource() {
  MVPN_COUNT_DTOR(TaskDownloadResource);
}

void TaskDownloadResource::run() {
  logger.debug() << "Download resource";

  NetworkRequest* request =
      NetworkRequest::createForGetUrl(this, m_fileUrl.toString());

  connect(request, &NetworkRequest::requestUpdated, this,
          &TaskDownloadResource::onProgress);
  connect(request, &NetworkRequest::requestCompleted, this,
          &TaskDownloadResource::onCompleted);
  connect(request, &NetworkRequest::requestFailed, this,
          &TaskDownloadResource::onFailed);
}

void TaskDownloadResource::onProgress(qint64 bytesReceived, qint64 bytesTotal) {
  logger.debug() << "Download progressed";

  Q_UNUSED(bytesTotal);

  emit progressed(bytesReceived);
}

void TaskDownloadResource::onCompleted(const QByteArray& data) {
  logger.debug() << "Download completed";

  emit completed(data, false);
}

void TaskDownloadResource::onFailed(QNetworkReply::NetworkError error,
                                    const QByteArray& data) {
  logger.debug() << "Download failed";

  bool hasError = error != QNetworkReply::NoError;

  emit completed(data, hasError);
}
