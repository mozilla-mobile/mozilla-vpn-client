/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controllerimpl.h"
#include "helper.h"
#include "ipaddress.h"
#include "mozillavpn.h"
#include "pinghelper.h"

Controller::Controller() {}

Controller::~Controller() = default;

void Controller::initialize() {}

void Controller::implInitialized(bool, bool, const QDateTime&) {}

void Controller::implPermRequired() {}

bool Controller::activate(const ServerData&, ActivationPrincipal user,
                          ServerSelectionPolicy) {
  return false;
}

bool Controller::switchServers(const ServerData& serverData) { return false; }

bool Controller::silentSwitchServers(ServerCoolDownPolicyForSilentSwitch) {
  return false;
}

bool Controller::silentServerSwitchingSupported() const { return false; }

void Controller::activateInternal(DNSPortPolicy, ServerSelectionPolicy,
                                  ActivationPrincipal user) {}

bool Controller::deactivate(ActivationPrincipal user) {
  Q_UNUSED(user);
  return false;
}

void Controller::connected(const QString& pubkey) { Q_UNUSED(pubkey); }

void Controller::disconnected() {}

void Controller::setState(State) {}

qint64 Controller::connectionTimestamp() const { return 42; }

void Controller::statusUpdated(const QString&, const QString&, uint64_t,
                               uint64_t) {}

QList<IPAddress> Controller::getAllowedIPAddressRanges(const Server& server) {
  Q_UNUSED(server);
  return QList<IPAddress>();
}

Controller::State Controller::state() const {
  return TestHelper::controllerState;
}

void Controller::updateRequired() {}

void Controller::getStatus(
    std::function<void(const QString& serverIpv4Gateway,
                       const QString& deviceIpv4Address, uint64_t txBytes,
                       uint64_t rxBytes)>&& a_callback) {
  std::function<void(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes)>
      callback = std::move(a_callback);
  callback("127.0.0.1", "127.0.0.1", 0, 0);
}

void Controller::quit() {}

void Controller::captivePortalPresent() {}

void Controller::captivePortalGone() {}

void Controller::handshakeTimeout() {}

QString Controller::currentServerString() const { return QString("42"); }

void Controller::logSerialize(QIODevice* device) { device->close(); }

void Controller::forceDaemonCrash() {}

void Controller::forceDaemonSilentServerSwitch() {}

void Controller::handleBackendFailure(ErrorCode code) {}
