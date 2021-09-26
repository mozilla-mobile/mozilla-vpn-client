/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/core.h"
#include "../../src/task.h"
#include "helper.h"

// The singleton.
static Core* s_instance = nullptr;

// static
Core* Core::instance() {
  if (!s_instance) {
    s_instance = new Core();
  }

  return s_instance;
}

Core::Core() {}

Core::~Core() {}

Core::State Core::state() const { return TestHelper::coreState; }

bool Core::stagingMode() const { return true; }

void Core::initialize() {}

void Core::setState(State) {}

void Core::getStarted() {}

void Core::authenticate() {}
void Core::authenticateWithType(Core::AuthenticationType) {}

void Core::openLink(LinkType) {}

void Core::scheduleTask(Task* task) {
  connect(task, &Task::completed, task, &Task::deleteLater);
  task->run(this);
}

void Core::maybeRunTask() {}

void Core::deleteTasks() {}

void Core::setToken(const QString&) {}

void Core::authenticationCompleted(const QByteArray&, const QString&) {}

void Core::deviceAdded(const QString&, const QString&, const QString&) {}

void Core::deviceRemoved(const QString&) {}

bool Core::setServerList(const QByteArray&) { return true; }

void Core::serversFetched(const QByteArray&) {}

void Core::removeDeviceFromPublicKey(const QString&) {}

void Core::accountChecked(const QByteArray&) {}

void Core::cancelAuthentication() {}

void Core::logout() {}

void Core::setAlert(AlertType) {}

void Core::errorHandle(ErrorHandler::ErrorType) {}

const QList<Server> Core::exitServers() const { return QList<Server>(); }

const QList<Server> Core::entryServers() const { return QList<Server>(); }

void Core::changeServer(const QString&, const QString&, const QString&,
                        const QString&) {}

void Core::postAuthenticationCompleted() {}

void Core::telemetryPolicyCompleted() {}

void Core::setUpdateRecommended(bool) {}

void Core::setUserAuthenticated(bool) {}

void Core::startSchedulingPeriodicOperations() {}

void Core::stopSchedulingPeriodicOperations() {}

bool Core::writeAndShowLogs(QStandardPaths::StandardLocation) { return true; }

bool Core::writeLogs(QStandardPaths::StandardLocation,
                     std::function<void(const QString& filename)>&&) {
  return true;
}

void Core::viewLogs() {}

bool Core::modelsInitialized() const { return true; }

void Core::taskCompleted() {}

void Core::requestSettings() {}

void Core::requestAbout() {}

void Core::requestViewLogs() {}

void Core::requestContactUs() {}

void Core::retrieveLogs() {}

void Core::storeInClipboard(const QString&) {}

void Core::cleanupLogs() {}

void Core::serializeLogs(QTextStream*, std::function<void()>&&) {}

void Core::activate() {}

void Core::deactivate() {}

void Core::refreshDevices() {}

void Core::quit() {}

void Core::update() {}

void Core::setUpdating(bool) {}

void Core::controllerStateChanged() {}

void Core::backendServiceRestore() {}

void Core::heartbeatCompleted(bool) {}

void Core::triggerHeartbeat() {}

void Core::submitFeedback(const QString&, const qint8, const QString&) {}

void Core::createSupportTicket(const QString&, const QString&, const QString&,
                               const QString&) {}

void Core::addCurrentDeviceAndRefreshData() {}

void Core::openAppStoreReviewLink() {}

bool Core::validateUserDNS(const QString&) const { return false; }

void Core::reset(bool) {}

void Core::maybeRegenerateDeviceKey() {}
