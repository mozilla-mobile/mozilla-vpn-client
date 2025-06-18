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
  ~NetworkManager();

  static NetworkManager* instance();

  static bool exists();

  static QByteArray osVersion();
  static QByteArray userAgent();

  QNetworkAccessManager* networkAccessManager();

  void clearCache();

  void increaseNetworkRequestCount();
  void decreaseNetworkRequestCount();

 protected:
  void clearCacheInternal();

 private:
  void localhostRequestsOnlyChanged();

 private:
  uint32_t m_requestCount = 0;
  bool m_clearCacheNeeded = false;
  QNetworkAccessManager* m_networkManager = nullptr;
};

#endif  // NETWORKMANAGER_H
