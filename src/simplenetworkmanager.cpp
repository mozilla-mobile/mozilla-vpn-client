/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "simplenetworkmanager.h"

#include <QNetworkAccessManager>

#include "utilities/leakdetector.h"

SimpleNetworkManager::SimpleNetworkManager() {
  MZ_COUNT_CTOR(SimpleNetworkManager);
}

SimpleNetworkManager::~SimpleNetworkManager() {
  MZ_COUNT_DTOR(SimpleNetworkManager);
}

QNetworkAccessManager* SimpleNetworkManager::networkAccessManager() {
  if (!m_networkManager) {
    m_networkManager = new QNetworkAccessManager(this);
  }

  return m_networkManager;
}

void SimpleNetworkManager::clearCacheInternal() {
  if (!m_networkManager) {
    return;
  }

  // We do not use credential access, but...
  m_networkManager->clearAccessCache();
  m_networkManager->clearConnectionCache();
}
