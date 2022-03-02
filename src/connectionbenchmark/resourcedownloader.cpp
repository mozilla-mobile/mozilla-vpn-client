/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "resourcedownloader.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "tasks/function/taskfunction.h"

#include <QUrl>

namespace {
Logger logger(LOG_NETWORKING, "ResourceDownloader");
}

ResourceDownloader::ResourceDownloader(const QUrl& fileUrl, QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(ResourceDownloader);

  TaskFunction* task =
      new TaskFunction([&]() { logger.debug() << "Downloader task"; });
  m_request = NetworkRequest::createForGetUrl(task, fileUrl.toString());

  connect(m_request, &NetworkRequest::requestCompleted, this,
          &ResourceDownloader::onCompleted);
  connect(m_request, &NetworkRequest::requestFailed, this,
          &ResourceDownloader::onFailed);
  connect(m_request, &NetworkRequest::requestUpdated, this,
          &ResourceDownloader::onProgress);
}

ResourceDownloader::~ResourceDownloader() {
  MVPN_COUNT_DTOR(ResourceDownloader);
}

void ResourceDownloader::onCompleted(const QByteArray& data) {
  logger.debug() << "Finished download" << data.size();

  m_downloadedData = data;

  emit downloaded();
}

void ResourceDownloader::onFailed(QNetworkReply::NetworkError error,
                                  const QByteArray& data) {
  logger.debug() << "Download failed" << error;

  m_downloadedData = data;

  emit aborted();
}

void ResourceDownloader::onProgress(qint64 bytesReceived, qint64 bytesTotal) {
  m_bytesReceived = bytesReceived;

  if (m_bytesTotal != bytesTotal) {
    m_bytesTotal = bytesTotal;
  }
}

void ResourceDownloader::abort() { m_request->abort(); }
