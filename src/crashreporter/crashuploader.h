/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHUPLOADER_H
#define CRASHUPLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class CrashUploader final : public QObject {
  Q_OBJECT
 public:
  explicit CrashUploader(QObject* parent = nullptr);
  void startUploads(QStringList files);
 signals:
  void uploadsComplete();

 private:
  void nextUpload();
  void startRequest(const QString& file);
  void requestComplete(QNetworkReply* reply);
  void dumpResponse(QNetworkReply* reply);
  QNetworkAccessManager* m_network;
  QStringList m_files;
  QString m_currentFile;
  uint16_t m_retries = 0;
};

#endif  // CRASHUPLOADER_H
