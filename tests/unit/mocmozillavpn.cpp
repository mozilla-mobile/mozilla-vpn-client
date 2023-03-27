/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"
#include "helper.h"
#include "models/location.h"
#include "models/servercountrymodel.h"
#include "models/subscriptiondata.h"
#include "mozillavpn.h"
#include "serverlatency.h"

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

MozillaVPN::MozillaVPN() : App(nullptr) {}

MozillaVPN::~MozillaVPN() {}

CaptivePortal* MozillaVPN::captivePortal() const { return nullptr; }

ConnectionHealth* MozillaVPN::connectionHealth() const { return nullptr; }

Controller* MozillaVPN::controller() const { return new Controller(); }

DeviceModel* MozillaVPN::deviceModel() const { return nullptr; }

Keys* MozillaVPN::keys() const { return nullptr; }

ServerData* MozillaVPN::serverData() const {
  static ServerData* data = new ServerData();
  return data;
}

ServerCountryModel* MozillaVPN::serverCountryModel() const {
  return new ServerCountryModel();
}

ServerLatency* MozillaVPN::serverLatency() const {
  static ServerLatency* serverLatency = new ServerLatency();
  return serverLatency;
}

SubscriptionData* MozillaVPN::subscriptionData() const {
  return new SubscriptionData();
}

User* MozillaVPN::user() const { return nullptr; }

Location* MozillaVPN::location() const {
  static Location* location = new Location();
  return location;
}

void MozillaVPN::initialize() {}

bool MozillaVPN::setServerList(QByteArray const&) { return true; }

void MozillaVPN::authenticate() {}
void MozillaVPN::authenticateWithType(
    AuthenticationListener::AuthenticationType) {}

void MozillaVPN::completeAuthentication(const QByteArray&, const QString&) {}

void MozillaVPN::deviceAdded(const QString&, const QString&, const QString&) {}

void MozillaVPN::removeDevice(const QString&, const QString&) {}

void MozillaVPN::deviceRemovalCompleted(const QString&) {}

void MozillaVPN::serversFetched(const QByteArray&) {}

void MozillaVPN::removeDeviceFromPublicKey(const QString&) {}

void MozillaVPN::accountChecked(const QByteArray&) {}

void MozillaVPN::cancelAuthentication() {}

void MozillaVPN::logout() {}

void MozillaVPN::postAuthenticationCompleted() {}

void MozillaVPN::mainWindowLoaded() {}

void MozillaVPN::telemetryPolicyCompleted() {}

void MozillaVPN::startSchedulingPeriodicOperations() {}

void MozillaVPN::stopSchedulingPeriodicOperations() {}

bool MozillaVPN::modelsInitialized() const { return true; }

void MozillaVPN::requestAbout() {}

void MozillaVPN::activate() {}

void MozillaVPN::deactivate() {}

void MozillaVPN::refreshDevices() {}

void MozillaVPN::update() {}

void MozillaVPN::setUpdating(bool) {}

void MozillaVPN::controllerStateChanged() {}

void MozillaVPN::backendServiceRestore() {}

void MozillaVPN::heartbeatCompleted(bool) {}

void MozillaVPN::triggerHeartbeat() {}

void MozillaVPN::submitFeedback(const QString&, const qint8, const QString&) {}

void MozillaVPN::createSupportTicket(const QString&, const QString&,
                                     const QString&, const QString&) {}

void MozillaVPN::addCurrentDeviceAndRefreshData(bool refreshProducts) {}

bool MozillaVPN::validateUserDNS(const QString&) const { return false; }

void MozillaVPN::reset(bool) {}

void MozillaVPN::maybeRegenerateDeviceKey() {}

void MozillaVPN::hardResetAndQuit() {}

void MozillaVPN::hardReset() {}

void MozillaVPN::requestDeleteAccount() {}

void MozillaVPN::cancelReauthentication() {}

void MozillaVPN::updateViewShown() {}

void MozillaVPN::setJournalPublicAndPrivateKeys(const QString&,
                                                const QString&) {}
void MozillaVPN::resetJournalPublicAndPrivateKeys() {}
bool MozillaVPN::checkCurrentDevice() { return true; }

void MozillaVPN::scheduleRefreshDataTasks(bool refreshProducts) {}

void MozillaVPN::registerUrlOpenerLabels() {}

// static
QString MozillaVPN::appVersionForUpdate() { return "42"; }
