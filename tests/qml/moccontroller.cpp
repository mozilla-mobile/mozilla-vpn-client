/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controllerimpl.h"
#include "ipaddress.h"
#include "mozillavpn.h"
#include "pinghelper.h"

ConnectionManager::Controller() {}

ConnectionManager::~Controller() = default;

void ConnectionManager::initialize() {}

void ConnectionManager::implInitialized(bool, bool, const QDateTime&) {}

bool ConnectionManager::activate(const ServerData&, ServerSelectionPolicy) {
  return false;
}

bool ConnectionManager::switchServers(const ServerData& serverData) { return false; }

bool ConnectionManager::silentSwitchServers(ServerCoolDownPolicyForSilentSwitch) {
  return false;
}

bool ConnectionManager::silentServerSwitchingSupported() const { return false; }

void ConnectionManager::activateInternal(DNSPortPolicy, ServerSelectionPolicy) {}

bool ConnectionManager::deactivate() { return false; }

void ConnectionManager::connected(const QString& pubkey,
                           const QDateTime& connectionTimestamp) {
  Q_UNUSED(pubkey);
  Q_UNUSED(connectionTimestamp);
}

void ConnectionManager::disconnected() {}

void ConnectionManager::timerTimeout() {}

void ConnectionManager::logout() {}

bool ConnectionManager::processNextStep() { return false; }

void ConnectionManager::setState(State) {}

qint64 ConnectionManager::time() const { return 42; }

void ConnectionManager::statusUpdated(const QString&, const QString&, uint64_t,
                               uint64_t) {}

QList<IPAddress> ConnectionManager::getAllowedIPAddressRanges(const Server& server) {
  Q_UNUSED(server);
  return QList<IPAddress>();
}

ConnectionManager::State ConnectionManager::state() const {
  return ConnectionManager::StateOff;
}

void ConnectionManager::updateRequired() {}

void ConnectionManager::getStatus(
    std::function<void(const QString& serverIpv4Gateway,
                       const QString& deviceIpv4Address, uint64_t txBytes,
                       uint64_t rxBytes)>&& a_callback) {
  std::function<void(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes)>
      callback = std::move(a_callback);
  callback("127.0.0.1", "127.0.0.1", 0, 0);
}

void ConnectionManager::quit() {}

void ConnectionManager::backendFailure() {}

void ConnectionManager::captivePortalPresent() {}

void ConnectionManager::captivePortalGone() {}

void ConnectionManager::handshakeTimeout() {}

#ifdef MZ_DUMMY
QString ConnectionManager::currentServerString() const { return QString("42"); }
#endif

void ConnectionManager::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&) {}
