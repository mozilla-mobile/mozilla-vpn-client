/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/mozillavpn.h"
#include "../../src/task.h"

// The singleton.
static MozillaVPN* s_instance = nullptr;

// static
MozillaVPN* MozillaVPN::instance() {
  if (!s_instance) {
    s_instance = new MozillaVPN();
  }

  return s_instance;
}

MozillaVPN::MozillaVPN() {}

MozillaVPN::~MozillaVPN() {}

MozillaVPN::State MozillaVPN::state() const { return StateInitialize; }
bool MozillaVPN::stagingMode() const { return true; }

void MozillaVPN::initialize() {}

void MozillaVPN::setState(State) {}

void MozillaVPN::getStarted() {}

void MozillaVPN::authenticate(MozillaVPN::AuthenticationType) {}

void MozillaVPN::openLink(LinkType) {}

void MozillaVPN::scheduleTask(Task* task) {
  connect(task, &Task::completed, task, &Task::deleteLater);
  task->run(this);
}

void MozillaVPN::maybeRunTask() {}

void MozillaVPN::deleteTasks() {}

void MozillaVPN::setToken(const QString&) {}

void MozillaVPN::authenticationCompleted(const QByteArray&, const QString&) {}

void MozillaVPN::deviceAdded(const QString&, const QString&, const QString&) {}

void MozillaVPN::deviceRemoved(const QString&) {}

bool MozillaVPN::setServerList(const QByteArray&) { return true; }

void MozillaVPN::serversFetched(const QByteArray&) {}

void MozillaVPN::removeDeviceFromPublicKey(const QString&) {}

void MozillaVPN::accountChecked(const QByteArray&) {}

void MozillaVPN::cancelAuthentication() {}

void MozillaVPN::logout() {}

void MozillaVPN::setAlert(AlertType) {}

void MozillaVPN::errorHandle(ErrorHandler::ErrorType) {}

const QList<Server> MozillaVPN::servers() const { return QList<Server>(); }

void MozillaVPN::changeServer(const QString&, const QString&) {}

void MozillaVPN::postAuthenticationCompleted() {}

void MozillaVPN::telemetryPolicyCompleted() {}

void MozillaVPN::setUpdateRecommended(bool) {}

void MozillaVPN::setUserAuthenticated(bool) {}

void MozillaVPN::startSchedulingPeriodicOperations() {}

void MozillaVPN::stopSchedulingPeriodicOperations() {}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation) {
  return true;
}

bool MozillaVPN::writeLogs(QStandardPaths::StandardLocation,
                           std::function<void(const QString& filename)>&&) {
  return true;
}

void MozillaVPN::viewLogs() {}

bool MozillaVPN::modelsInitialized() const { return true; }

void MozillaVPN::taskCompleted() {}

void MozillaVPN::requestSettings() {}

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

void MozillaVPN::addCurrentDeviceAndRefreshData() {}

void MozillaVPN::abortAuthentication() {}

void MozillaVPN::appReviewRequested() {}
