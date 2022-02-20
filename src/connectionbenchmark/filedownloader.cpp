/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "filedownloader.h"
#include "leakdetector.h"
#include "logger.h"

#include <QDebug>
#include <QUrl>

FileDownloader::FileDownloader(QUrl fileUrl, QObject *parent) : QObject(parent) {
  MVPN_COUNT_CTOR(FileDownloader);

  QNetworkRequest request(fileUrl);
  m_networkReply = m_networkAccessManager.get(request);

  connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this,
          &FileDownloader::onFinished);
  connect(m_networkReply, &QNetworkReply::downloadProgress, this,
          &FileDownloader::onDownloadProgress);
}

FileDownloader::~FileDownloader() { MVPN_COUNT_DTOR(FileDownloader); }

void FileDownloader::onFinished(QNetworkReply* pReply) {
  m_downloadedData = pReply->readAll();
  pReply->deleteLater();

  if (pReply->error() == QNetworkReply::OperationCanceledError) {
    emit aborted();
  } else {
    emit downloaded();
  }
}

void FileDownloader::onDownloadProgress(qint64 bytesReceived,
                                        qint64 bytesTotal) {
  m_bytesReceived = bytesReceived;
  m_bytesTotal = bytesTotal;
}

void FileDownloader::abort() { m_networkReply->abort(); }

QByteArray FileDownloader::downloadedData() const { return m_downloadedData; }
