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
  logger.debug() << "Run download resource";

  m_request = NetworkRequest::createForGetUrl(this, m_fileUrl.toString());

  connect(m_request, &NetworkRequest::requestUpdated, this,
          &TaskDownloadResource::taskProgressed);
  connect(m_request, &NetworkRequest::requestCompleted, this,
          &TaskDownloadResource::taskCompleted);
  connect(m_request, &NetworkRequest::requestFailed, this,
          &TaskDownloadResource::taskFailed);
}

void TaskDownloadResource::stop() {
  logger.debug() << "Stop download resource";
  Q_ASSERT(m_request);

  m_request->abort();
}

void TaskDownloadResource::taskProgressed(qint64 bytesReceived,
                                          qint64 bytesTotal) {
  logger.debug() << "Download progressed";

  Q_UNUSED(bytesTotal);

  emit progressed(bytesReceived);
}

void TaskDownloadResource::taskCompleted(const QByteArray& data) {
  logger.debug() << "Download completed";

  emit completed(data, false);
}

void TaskDownloadResource::taskFailed(QNetworkReply::NetworkError error,
                                      const QByteArray& data) {
  logger.debug() << "Download failed";

  bool hasError = error != QNetworkReply::NoError;

  emit completed(data, hasError);
}
