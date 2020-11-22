/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanager.h"
#include "leakdetector.h"

// TODO: constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char* API_URL_PROD =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

#ifdef QT_DEBUG
constexpr const char* API_URL_DEBUG =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";
#endif

namespace {
NetworkManager* s_instance = nullptr;
}

NetworkManager::NetworkManager() {
  MVPN_COUNT_CTOR(NetworkManager);

  Q_ASSERT(!s_instance);
  s_instance = this;

  // API URL depends on the type of build.
  m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
  m_apiUrl = API_URL_DEBUG;
#endif
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
