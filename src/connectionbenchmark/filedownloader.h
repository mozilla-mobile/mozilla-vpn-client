/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class FileDownloader : public QObject {
  Q_OBJECT

  public:
    explicit FileDownloader(QUrl fileUrl, QObject *parent = 0);

    virtual ~FileDownloader();

    QByteArray downloadedData() const;

    void abort();

    qint64 bytesReceived() { return m_bytesReceived; }

   signals:
    void downloaded();
    void aborted();

   private slots:
    void onFinished(QNetworkReply* pReply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

   private:
    QNetworkAccessManager m_networkAccessManager;
    QNetworkReply* m_networkReply;
    QByteArray m_downloadedData;
    qint64 m_bytesReceived;
    qint64 m_bytesTotal;
};

#endif // FILEDOWNLOADER_H