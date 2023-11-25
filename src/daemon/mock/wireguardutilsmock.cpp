/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilsmock.h"

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
  Q_UNUSED(config);
  return true;
}

bool WireguardUtilsMock::deleteInterface() {
  return true;
}

// dummy implementations for now
bool WireguardUtilsMock::updatePeer(const InterfaceConfig& config) {
  return true;
}

bool WireguardUtilsMock::deletePeer(const InterfaceConfig& config) {
  return true;
}

QList<WireguardUtils::PeerStatus> WireguardUtilsMock::getPeerStatus() {
  QList<PeerStatus> peerList;
  return peerList;
}

bool WireguardUtilsMock::updateRoutePrefix(const IPAddress& prefix) {
  return false;
}

bool WireguardUtilsMock::deleteRoutePrefix(const IPAddress& prefix) {
  return false;
}

bool WireguardUtilsMock::addExclusionRoute(const IPAddress& prefix) {
  return true;
}

bool WireguardUtilsMock::deleteExclusionRoute(const IPAddress& prefix) {
  return true;
}
