/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "resourcedownloader.h"
#include "leakdetector.h"
#include "logger.h"

#include <QUrl>

ResourceDownloader::ResourceDownloader(const QUrl& fileUrl, QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(ResourceDownloader);

  QNetworkRequest request(fileUrl);
  m_networkReply = m_networkAccessManager.get(request);

  connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this,
          &ResourceDownloader::onFinished);
  connect(m_networkReply, &QNetworkReply::downloadProgress, this,
          &ResourceDownloader::onDownloadProgress);
}

ResourceDownloader::~ResourceDownloader() {
  MVPN_COUNT_DTOR(ResourceDownloader);
}

void ResourceDownloader::onFinished(QNetworkReply* reply) {
  m_downloadedData = reply->readAll();
  reply->deleteLater();

  if (reply->error() == QNetworkReply::OperationCanceledError) {
    emit aborted();
  } else {
    emit downloaded();
  }
}

void ResourceDownloader::onDownloadProgress(qint64 bytesReceived,
                                            qint64 bytesTotal) {
  m_bytesReceived = bytesReceived;

  if (m_bytesTotal != bytesTotal) {
    m_bytesTotal = bytesTotal;
  }
}

void ResourceDownloader::abort() { m_networkReply->abort(); }
