/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BALROG_H
#define BALROG_H

#include "errorhandler.h"
#include "updater.h"
#include "temporarydir.h"

#include <QCryptographicHash>
#include <QNetworkReply>

class NetworkRequest;

class Balrog final : public Updater {
  Q_DISABLE_COPY_MOVE(Balrog)

 public:
  Balrog(QObject* parent, bool downloadAndInstall,
         ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~Balrog();

  void start(Task* task) override;

 private:
  static QString userAgent();

  bool processData(Task* task, const QByteArray& data);
  bool fetchSignature(Task* task, NetworkRequest* request,
                      const QByteArray& data);
  bool checkSignature(Task* task, const QByteArray& x5uData,
                      const QByteArray& updateData,
                      const QByteArray& signatureBlob);
  bool validateSignature(const QByteArray& x5uData,
                         const QByteArray& updateData,
                         const QByteArray& signatureBlob);
  bool computeHash(const QString& url, const QByteArray& data,
                   const QString& hashValue, const QString& hashFunction);
  bool saveFileAndInstall(const QString& url, const QByteArray& data);
  bool install(const QString& filePath);
  void propagateError(NetworkRequest* request,
                      QNetworkReply::NetworkError error);

 private:
  TemporaryDir m_tmpDir;
  bool m_downloadAndInstall;
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // BALROG_H
