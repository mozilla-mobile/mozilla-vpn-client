/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/controllerimpl.h"
#include "../../src/ipaddressrange.h"
#include "../../src/mozillavpn.h"
#include "helper.h"

Controller::Controller() {}

Controller::~Controller() = default;

void Controller::initialize() {}

void Controller::implInitialized(bool, bool, const QDateTime&) {}

void Controller::activate() {}

void Controller::activateInternal() {}

void Controller::deactivate() {}

void Controller::connected() {}

void Controller::disconnected() {}

void Controller::timerTimeout() {}

void Controller::changeServer(const QString&, const QString&) {}

void Controller::logout() {}

bool Controller::processNextStep() { return false; }

void Controller::setState(State) {}

int Controller::time() const { return 42; }

void Controller::getBackendLogs(std::function<void(const QString&)>&&) {}

void Controller::statusUpdated(const QString&, uint64_t, uint64_t) {}

QList<IPAddressRange> Controller::getAllowedIPAddressRanges(
    const Server& server) {
  Q_UNUSED(server);
  return QList<IPAddressRange>();
}

Controller::State Controller::state() const {
  return TestHelper::controllerState;
}

void Controller::updateRequired() {}

void Controller::getStatus(
    std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                       uint64_t rxBytes)>&& a_callback) {
  std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                     uint64_t rxBytes)>
      callback = std::move(a_callback);
  callback("127.0.0.1", 0, 0);
}

void Controller::quit() {}

void Controller::connectionConfirmed() {}

void Controller::connectionFailed() {}
