/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SIMPLENETWORKMANAGER_H
#define SIMPLENETWORKMANAGER_H

#include <QObject>

#include "networkmanager.h"

class SimpleNetworkManager final : public NetworkManager {
  Q_DISABLE_COPY_MOVE(SimpleNetworkManager)

 public:
  SimpleNetworkManager();
  ~SimpleNetworkManager();

  QNetworkAccessManager* networkAccessManager() override;

 protected:
  void clearCacheInternal() override;

 private:
  QNetworkAccessManager* m_networkManager = nullptr;
};

#endif  // SIMPLENETWORKMANAGER_H
