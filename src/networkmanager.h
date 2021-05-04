/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>

class QNetworkAccessManager;

class NetworkManager : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkManager)

 public:
  NetworkManager();
  virtual ~NetworkManager();

  static NetworkManager* instance();

  static QByteArray userAgent();

  virtual QNetworkAccessManager* networkAccessManager() = 0;

  virtual void clearCache() = 0;
};

#endif  // NETWORKMANAGER_H
