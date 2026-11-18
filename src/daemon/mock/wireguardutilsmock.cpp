/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilsmock.h"

#include <QDateTime>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("WireguardUtilsMock");
};  // namespace

WireguardUtilsMock::WireguardUtilsMock(QObject* parent)
    : WireguardUtils(parent) {
  MZ_COUNT_CTOR(WireguardUtilsMock);
  logger.debug() << "WireguardUtilsMock created.";
}

WireguardUtilsMock::~WireguardUtilsMock() {
  MZ_COUNT_DTOR(WireguardUtilsMock);
  logger.debug() << "WireguardUtilsMock destroyed.";
}

bool WireguardUtilsMock::addInterface(const InterfaceConfig& config) {
  if (m_config.isValid()) {
    return false;
  }
  m_config = config;
  return m_config.isValid();
}

bool WireguardUtilsMock::deleteInterface() {
  // Normally, the interface going down would drop all routes associated
  // with it, but for the purpose of testing, only automatically delete
  // exclusion routes. This lets us test that the daemon handles refcounting
  // and garbage collection correctly.
  m_routes.removeIf([](QHash<IPAddress, RouteType>::iterator i){
    return i.value() == RouteExclusion;
  });

  m_handshakes.clear();
  m_config = InterfaceConfig();
  return true;
}

bool WireguardUtilsMock::updatePeer(const InterfaceConfig& config) {
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  m_handshakes[config.m_serverPublicKey] = now + m_handshakeDelayMsec;
  return true;
}

bool WireguardUtilsMock::deletePeer(const InterfaceConfig& config) {
  return m_handshakes.remove(config.m_serverPublicKey) > 0;
}

QList<WireguardUtils::PeerStatus> WireguardUtilsMock::getPeerStatus() {
  QList<PeerStatus> peerList;
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  for (const QString& pubkey : m_handshakes.keys()) {
    PeerStatus status(pubkey);
    qint64 hsTime = m_handshakes[pubkey];
    if (now > hsTime) {
      status.m_handshake = hsTime;
      status.m_rxBytes = (now - hsTime) * 7;
      status.m_txBytes = (now - hsTime) * 3;
    }
    peerList.append(status);
  }
  return peerList;
}

void WireguardUtilsMock::setRoutePrefix(const IPAddress& prefix, RouteType type) {
  m_routes[prefix] = type;
}

bool WireguardUtilsMock::updateRoutePrefix(const IPAddress& prefix) {
  if (m_routes.value(prefix, RoutePeer) != RoutePeer) {
    return false;
  }
  setRoutePrefix(prefix, RoutePeer);
  return true;
}

bool WireguardUtilsMock::deleteRoutePrefix(const IPAddress& prefix) {
  return m_routes.remove(prefix);
}

bool WireguardUtilsMock::excludeLocalNetworks(const QList<IPAddress>& addrs) {
  for (const IPAddress& prefix : addrs) {
    if (m_routes.contains(prefix)) {
      return false;
    }
    setRoutePrefix(prefix, RouteExclusion);
  }

  return true;
}
