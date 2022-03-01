/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RESOURCEDOWNLOADER_H
#define RESOURCEDOWNLOADER_H

#include "networkrequest.h"

#include <QObject>
#include <QByteArray>
#include <QNetworkReply>
#include <QUrl>

class ResourceDownloader final : public QObject {
  Q_OBJECT

 public:
  ResourceDownloader(const QUrl& fileUrl, QObject* parent);
  ~ResourceDownloader();

  QByteArray downloadedData() const { return m_downloadedData; }
  qint64 bytesReceived() const { return m_bytesReceived; }

  void abort();

 signals:
  void downloaded();
  void aborted();

 private slots:
  void onCompleted(const QByteArray& data);
  void onFailed(QNetworkReply::NetworkError error, const QByteArray& data);
  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

 private:
  NetworkRequest* m_request;

  QByteArray m_downloadedData;
  qint64 m_bytesReceived;
  qint64 m_bytesTotal;
};

#endif  // RESOURCEDOWNLOADER_H
