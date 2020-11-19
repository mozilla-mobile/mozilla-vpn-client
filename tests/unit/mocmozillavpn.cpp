/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/mozillavpn.h"
#include "../../src/task.h"
#include "helper.h"

// The singleton.
static MozillaVPN *s_instance = nullptr;

// static
MozillaVPN *MozillaVPN::instance()
{
    if (!s_instance) {
        s_instance = new MozillaVPN();
    }

    return s_instance;
}

MozillaVPN::MozillaVPN() {}

MozillaVPN::~MozillaVPN() {}

MozillaVPN::State MozillaVPN::state() const
{
    return  TestHelper::vpnState;
}

void MozillaVPN::initialize()
{
}

void MozillaVPN::setState(State)
{
}

void MozillaVPN::authenticate()
{
}

void MozillaVPN::openLink(LinkType)
{
}

void MozillaVPN::scheduleTask(Task *task)
{
    task->run(this);
    connect(task, &Task::completed, task, &Task::deleteLater);
}

void MozillaVPN::maybeRunTask()
{
}

void MozillaVPN::deleteTasks() {}

void MozillaVPN::setToken(const QString &) 
{
}

void MozillaVPN::authenticationCompleted(const QByteArray &, const QString &)
{
}

void MozillaVPN::deviceAdded(const QString &, const QString &, const QString &)
{
}

void MozillaVPN::deviceRemoved(const QString &)
{
}

bool MozillaVPN::setServerList(const QByteArray &)
{
    return true;
}

void MozillaVPN::serversFetched(const QByteArray &)
{
}

void MozillaVPN::removeDevice(const QString &)
{
}

void MozillaVPN::accountChecked(const QByteArray &)
{
}

void MozillaVPN::cancelAuthentication()
{
}

void MozillaVPN::logout()
{
}

void MozillaVPN::setAlert(AlertType)
{
}

void MozillaVPN::errorHandle(ErrorHandler::ErrorType)
{
}

const QList<Server> MozillaVPN::getServers() const
{
    return QList<Server>();
}

void MozillaVPN::changeServer(const QString &, const QString &)
{
}

void MozillaVPN::postAuthenticationCompleted()
{
}

void MozillaVPN::setUpdateRecommended(bool)
{
}

void MozillaVPN::setUserAuthenticated(bool)
{
}

void MozillaVPN::startSchedulingPeriodicOperations() {}

void MozillaVPN::stopSchedulingPeriodicOperations() {}

void MozillaVPN::subscribe()
{
}

void MozillaVPN::restoreSubscription() {}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation)
{
    return true;
}

bool MozillaVPN::writeLogs(QStandardPaths::StandardLocation,
                           std::function<void(const QString &filename)> &&)
{
    return true;
}

void MozillaVPN::viewLogs()
{
}

bool MozillaVPN::modelsInitialized() const
{
    return true;
}

void MozillaVPN::taskCompleted() {}

void MozillaVPN::requestSettings() {}

void MozillaVPN::requestAbout() {}

void MozillaVPN::requestViewLogs() {}

QString MozillaVPN::retrieveLogs()
{
    return "LOGS!";
}

void MozillaVPN::storeInClipboard(const QString &) {}

bool MozillaVPN::startOnBootSupported() const { return false; }

void MozillaVPN::activate() {}

void MozillaVPN::deactivate() {}

void MozillaVPN::refreshDevices() {}
