/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/controllerimpl.h"
#include "../../src/ipaddress.h"
#include "../../src/mozillavpn.h"
#include "helper.h"

Controller::Controller() {}

Controller::~Controller() = default;

void Controller::initialize() {}

void Controller::implInitialized(bool, bool, const QDateTime&) {}

bool Controller::activate() { return false; }

void Controller::activateInternal() {}

bool Controller::deactivate() { return false; }

void Controller::connected(const QString& pubkey) { Q_UNUSED(pubkey); }

void Controller::disconnected() {}

void Controller::timerTimeout() {}

void Controller::changeServer(const QString&, const QString&, const QString&,
                              const QString&) {}

void Controller::logout() {}

bool Controller::processNextStep() { return false; }

void Controller::setState(State) {}

qint64 Controller::time() const { return 42; }

void Controller::getBackendLogs(std::function<void(const QString&)>&&) {}

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

void Controller::backendFailure() {}

void Controller::captivePortalPresent() {}

void Controller::captivePortalGone() {}

QString Controller::currentLocalizedCityName() const { return ""; }

QString Controller::switchingLocalizedCityName() const { return ""; }

void Controller::handshakeTimeout() {}

void Controller::setCooldownForAllServersInACity(const QString& countryCode,
                                                 const QString& cityCode) {
  Q_UNUSED(countryCode);
  Q_UNUSED(cityCode);
}
