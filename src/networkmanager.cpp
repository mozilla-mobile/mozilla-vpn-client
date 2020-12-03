/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanager.h"
#include "leakdetector.h"

#ifdef MVPN_PRODUCTION_MODE
constexpr const char* API_URL = "https://fpn.firefox.com";
#else
constexpr const char* API_URL =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";
#endif

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
  userAgent.append(QSysInfo::productType().toLocal8Bit());
  userAgent.append(" ");
  userAgent.append(QSysInfo::productVersion().toLocal8Bit());
  userAgent.append(")");

  return userAgent;
}

QString NetworkManager::apiUrl() const { return API_URL; }
