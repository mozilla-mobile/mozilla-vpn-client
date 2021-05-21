/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanager.h"
#include "constants.h"
#include "leakdetector.h"

namespace {
NetworkManager* s_instance = nullptr;
}

NetworkManager::NetworkManager() {
  MVPN_COUNT_CTOR(NetworkManager);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

NetworkManager::~NetworkManager() {
  MVPN_COUNT_DTOR(NetworkManager);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
NetworkManager* NetworkManager::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
QByteArray NetworkManager::userAgent() {
  QByteArray userAgent;
  userAgent.append("MozillaVPN/" APP_VERSION " (");
#ifdef MVPN_WASM
  userAgent.append("WASM");
#else
  userAgent.append(QSysInfo::productType().toLocal8Bit());
  userAgent.append(" ");
  userAgent.append(QSysInfo::productVersion().toLocal8Bit());
  userAgent.append(")");
#endif

  return userAgent;
}

void NetworkManager::clearCache() {
  if (m_requestCount == 0) {
    Q_ASSERT(m_clearCacheNeeded == false);
    clearCacheInternal();
    return;
  }

  m_clearCacheNeeded = true;
}

void NetworkManager::increaseNetworkRequestCount() { ++m_requestCount; }

void NetworkManager::decreaseNetworkRequestCount() {
  Q_ASSERT(m_requestCount > 0);
  --m_requestCount;

  if (m_requestCount == 0 && m_clearCacheNeeded) {
    m_clearCacheNeeded = false;
    clearCacheInternal();
  }
}
