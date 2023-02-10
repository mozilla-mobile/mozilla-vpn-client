/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"
#include "helper.h"
#include "mozillavpn.h"

// The singleton.
static MozillaVPN* s_instance = nullptr;

// static
MozillaVPN* MozillaVPN::instance() { return maybeInstance(); }

// static
MozillaVPN* MozillaVPN::maybeInstance() {
  if (!s_instance) {
    s_instance = new MozillaVPN();
  }

  return s_instance;
}

MozillaVPN::MozillaVPN() {}

MozillaVPN::~MozillaVPN() {}

ConnectionHealth* MozillaVPN::connectionHealth() const { return nullptr; }

Controller* MozillaVPN::controller() const { return new Controller(); }

SubscriptionData* MozillaVPN::subscriptionData() const {
  return new SubscriptionData();
}

ServerCountryModel* MozillaVPN::serverCountryModel() const {
  return new ServerCountryModel();
}

ServerLatency* MozillaVPN::serverLatency() const {
  static ServerLatency* serverLatency = new ServerLatency();
  return serverLatency;
}

Location* MozillaVPN::location() const {
  static Location* location = new Location();
  return location;
}

MozillaVPN::State MozillaVPN::state() const { return StateInitialize; }
MozillaVPN::UserState MozillaVPN::userState() const {
  return UserNotAuthenticated;
}

bool MozillaVPN::stagingMode() const {
  return TestHelper::instance()->stagingMode();
}
bool MozillaVPN::debugMode() const {
  return TestHelper::instance()->debugMode();
}

void MozillaVPN::initialize() {}

void MozillaVPN::setState(State) {}

void MozillaVPN::authenticate() {}
void MozillaVPN::authenticateWithType(MozillaVPN::AuthenticationType) {}

void MozillaVPN::setToken(const QString&) {}

void MozillaVPN::authenticationCompleted(const QByteArray&, const QString&) {}

void MozillaVPN::deviceAdded(const QString&, const QString&, const QString&) {}

void MozillaVPN::deviceRemoved(const QString&, const QString&) {}

void MozillaVPN::deviceRemovalCompleted(const QString&) {}

void MozillaVPN::serversFetched(const QByteArray&) {}

void MozillaVPN::removeDeviceFromPublicKey(const QString&) {}

void MozillaVPN::accountChecked(const QByteArray&) {}

void MozillaVPN::cancelAuthentication() {}

void MozillaVPN::logout() {}

void MozillaVPN::postAuthenticationCompleted() {}

void MozillaVPN::mainWindowLoaded() {
  TestHelper::instance()->setMainWindowLoadedCalled(true);
}

void MozillaVPN::telemetryPolicyCompleted() {}

void MozillaVPN::setUserState(UserState) {}

void MozillaVPN::startSchedulingPeriodicOperations() {}

void MozillaVPN::stopSchedulingPeriodicOperations() {}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation) {
  return true;
}

bool MozillaVPN::writeLogs(QStandardPaths::StandardLocation,
                           std::function<void(const QString& filename)>&&) {
  return true;
}

bool MozillaVPN::viewLogs() { return true; }

bool MozillaVPN::modelsInitialized() const { return true; }

void MozillaVPN::requestAbout() {}

void MozillaVPN::requestViewLogs() {}

void MozillaVPN::retrieveLogs() {}

void MozillaVPN::storeInClipboard(const QString&) {}

void MozillaVPN::cleanupLogs() {}

void MozillaVPN::serializeLogs(QTextStream*, std::function<void()>&&) {}

void MozillaVPN::activate() {}

void MozillaVPN::deactivate() {}

void MozillaVPN::refreshDevices() {}

void MozillaVPN::quit() {}

void MozillaVPN::update() {}

void MozillaVPN::setUpdating(bool) {}

void MozillaVPN::controllerStateChanged() {}

void MozillaVPN::backendServiceRestore() {}

void MozillaVPN::heartbeatCompleted(bool) {}

void MozillaVPN::triggerHeartbeat() {}

void MozillaVPN::submitFeedback(const QString&, const qint8, const QString&) {}

void MozillaVPN::createSupportTicket(const QString&, const QString&,
                                     const QString&, const QString&) {}

void MozillaVPN::addCurrentDeviceAndRefreshData(bool refreshData) {}

void MozillaVPN::abortAuthentication() {}

void MozillaVPN::openAppStoreReviewLink() {}

bool MozillaVPN::validateUserDNS(const QString&) const { return false; }

void MozillaVPN::reset(bool) {}

void MozillaVPN::maybeRegenerateDeviceKey() {}

void MozillaVPN::hardResetAndQuit() {}

void MozillaVPN::hardReset() {}

void MozillaVPN::exitForUnrecoverableError(const QString& reason) {}

void MozillaVPN::crashTest() {}

QString MozillaVPN::devVersion() { return qVersion(); }

QString MozillaVPN::graphicsApi() { return ""; }

void MozillaVPN::requestDeleteAccount() {}

void MozillaVPN::cancelReauthentication() {}

void MozillaVPN::updateViewShown() {}

void MozillaVPN::scheduleRefreshDataTasks(bool refreshProducts) {}

void MozillaVPN::registerUrlOpenerLabels() {}
