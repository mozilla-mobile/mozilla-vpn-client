/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanager.h"

#include "context/constants.h"
#include "networkrequest.h"
#include "settings/settingsholder.h"
#include "utilities/leakdetector.h"

#if MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif

#include <QTextStream>

namespace {
NetworkManager* s_instance = nullptr;

bool localhostRequestCallback(NetworkRequest* request) {
  QString host(request->url().host());
  if (host == "localhost" || host == "127.0.0.1" || host == "::1") {
    return false;
  }

  qFatal("Non localhost request detected!");
  Q_ASSERT(false);
  return true;
}
}  // namespace

NetworkManager::NetworkManager() {
  MZ_COUNT_CTOR(NetworkManager);

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(SettingsHolder::instance(),
          &SettingsHolder::localhostRequestsOnlyChanged, this,
          &NetworkManager::localhostRequestsOnlyChanged);

  localhostRequestsOnlyChanged();
}

NetworkManager::~NetworkManager() {
  MZ_COUNT_DTOR(NetworkManager);

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
#ifdef MZ_WASM
    out << "WASM";
#elif MZ_WINDOWS
    out << QSysInfo::productType().toLocal8Bit() << " "
        << WindowsUtils::windowsVersion().toLocal8Bit();
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
    flags.append("iap:true");

    QTextStream out(&userAgent);
    out << Constants::NETWORK_USERAGENT_PREFIX << "/"
        << Constants::versionString() << " (" << flags.join("; ") << ")";
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

void NetworkManager::localhostRequestsOnlyChanged() {
  if (SettingsHolder::instance()->localhostRequestsOnly()) {
    NetworkRequest::setRequestHandler(
        localhostRequestCallback, localhostRequestCallback,
        [](NetworkRequest* request, const QByteArray&) -> bool {
          return localhostRequestCallback(request);
        },
        [](NetworkRequest* request, QIODevice*) -> bool {
          return localhostRequestCallback(request);
        });
  }
}
