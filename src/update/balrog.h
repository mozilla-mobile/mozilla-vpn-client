/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BALROG_H
#define BALROG_H

#include "updater.h"

#include <QCryptographicHash>

class QNetworkReply;

class Balrog final : public Updater {
  Q_DISABLE_COPY_MOVE(Balrog)

 public:
  Balrog(QObject* parent);
  ~Balrog();

  void start() override;

 private:
  static QString userAgent();

  bool processData(const QByteArray& data);
  bool fetchSignature(QNetworkReply* reply, const QByteArray& data);
  bool checkSignature(const QByteArray& signature,
                      const QByteArray& signatureBlob,
                      QCryptographicHash::Algorithm algorithm,
                      const QByteArray& data);
  bool validateSignature(const QByteArray& publicKey, const QByteArray& data,
                         QCryptographicHash::Algorithm algorithm,
                         const QByteArray& signature);
  bool computeHash(const QString& url, const QByteArray& data,
                   const QString& hashValue, const QString& hashFunction);
  bool saveFileAndInstall(const QString& url, const QByteArray& data);
  bool install(const QString& filePath);
};

#endif  // BALROG_H
