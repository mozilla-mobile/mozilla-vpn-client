/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilsmock.h"

#include <QDateTime>

#include "leakdetector.h"
#include "logger.h"

constexpr int MOCK_HANDSHAKE_DELAY_MSEC = 1500;

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
  Q_UNUSED(config);
  return true;
}

bool WireguardUtilsMock::deleteInterface() { return true; }

bool WireguardUtilsMock::updatePeer(const InterfaceConfig& config) {
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  m_handshakes[config.m_serverPublicKey] = now + MOCK_HANDSHAKE_DELAY_MSEC;
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

bool WireguardUtilsMock::updateRoutePrefix(const IPAddress& prefix) {
  return true;
}

bool WireguardUtilsMock::deleteRoutePrefix(const IPAddress& prefix) {
  return true;
}

bool WireguardUtilsMock::addExclusionRoute(const IPAddress& prefix) {
  return true;
}

bool WireguardUtilsMock::deleteExclusionRoute(const IPAddress& prefix) {
  return true;
}
