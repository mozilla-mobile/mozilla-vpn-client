/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "models/feature.h"

#if MVPN_WINDOWS
#  include "platforms/windows/windowscommons.h"
#endif

#include <QTextStream>

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
  Q_ASSERT(exists());
  return s_instance;
}

// static
bool NetworkManager::exists() { return !!s_instance; }

// static
QByteArray NetworkManager::osVersion() {
  QByteArray osVersion;

  {
    QTextStream out(&osVersion);
#ifdef MVPN_WASM
    out << "WASM";
#elif MVPN_WINDOWS
    out << QSysInfo::productType().toLocal8Bit() << " "
        << WindowsCommons::WindowsVersion().toLocal8Bit();
#else
    out << QSysInfo::productType().toLocal8Bit() << " "
        << QSysInfo::productVersion().toLocal8Bit();
#endif
  }

  return osVersion;
}

// static
QByteArray NetworkManager::userAgent() {
  QByteArray userAgent;

  {
    QStringList flags;
    flags.append(QString("sys:") + NetworkManager::osVersion());

    if (Feature::get(Feature::Feature_inAppPurchase)->isSupported()) {
      flags.append("iap:true");
    }

#ifdef MVPN_EXTRA_USERAGENT
    flags.append(MVPN_EXTRA_USERAGENT);
#endif

    QTextStream out(&userAgent);
    out << "MozillaVPN/" << Constants::versionString() << " ("
        << flags.join("; ") << ")";
  }

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
