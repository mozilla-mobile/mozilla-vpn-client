/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcontroller.h"
#include "dbus.h"
#include "device.h"
#include "errorhandler.h"
#include "keys.h"
#include "logger.h"
#include "mozillavpn.h"
#include "server.h"

#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QString>

namespace {
Logger logger({LOG_LINUX, LOG_CONTROLLER}, "LinuxController");
}

LinuxController::LinuxController()
{
    m_dbus = new DBus(this);
    connect(m_dbus, &DBus::connected, this, &LinuxController::connected);
    connect(m_dbus, &DBus::disconnected, this, &LinuxController::disconnected);
}

void LinuxController::initialize(const Device *device, const Keys *keys)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);

    QDBusPendingCallWatcher *watcher = m_dbus->status();
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<QString> reply = *call;
                         if (reply.isError()) {
                             logger.log() << "Error received from the DBus service";
                             emit initialized(false, Controller::StateOff, QDateTime());
                             return;
                         }

                         QString status = reply.argumentAt<0>();
                         logger.log() << "Status:" << status;

                         QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
                         Q_ASSERT(json.isObject());

                         QJsonObject obj = json.object();
                         Q_ASSERT(obj.contains("status"));
                         QJsonValue statusValue = obj.take("status");
                         Q_ASSERT(statusValue.isBool());

                         emit initialized(true,
                                          statusValue.toBool() ? Controller::StateOn
                                                               : Controller::StateOff,
                                          QDateTime::currentDateTime());
                     });
}

void LinuxController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               const CaptivePortal &captivePortal,
                               const QList<IPAddressRange> &allowedIPAddressRanges,
                               bool forSwitching)
{
    Q_UNUSED(forSwitching);

    logger.log() << "LinuxController activated";
    monitorWatcher(m_dbus->activate(server, device, keys, captivePortal, allowedIPAddressRanges));
}

void LinuxController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);
    logger.log() << "LinuxController deactivated";
    monitorWatcher(m_dbus->deactivate());
}

void LinuxController::monitorWatcher(QDBusPendingCallWatcher *watcher)
{
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<bool> reply = *call;
                         if (reply.isError()) {
                             logger.log() << "Error received from the DBus service";
                             MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
                             emit disconnected();
                             return;
                         }

                         bool status = reply.argumentAt<0>();
                         if (status) {
                             logger.log() << "DBus service says: all good.";
                             // we will receive the connected/disconnected() signal;
                             return;
                         }

                         logger.log() << "DBus service says: error.";
                         MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
                         emit disconnected();
                     });
}

void LinuxController::checkStatus()
{
    logger.log() << "Check status";

    QDBusPendingCallWatcher *watcher = m_dbus->status();
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<QString> reply = *call;
                         if (reply.isError()) {
                             logger.log() << "Error received from the DBus service";
                             return;
                         }

                         QString status = reply.argumentAt<0>();
                         logger.log() << "Status:" << status;

                         QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
                         Q_ASSERT(json.isObject());

                         QJsonObject obj = json.object();
                         Q_ASSERT(obj.contains("status"));
                         QJsonValue statusValue = obj.take("status");
                         Q_ASSERT(statusValue.isBool());

                         if (!statusValue.toBool()) {
                             logger.log() << "Unable to retrieve the status from the interface.";
                             return;
                         }

                         Q_ASSERT(obj.contains("serverIpv4Gateway"));
                         QJsonValue serverIpv4Gateway = obj.take("serverIpv4Gateway");
                         Q_ASSERT(serverIpv4Gateway.isString());

                         Q_ASSERT(obj.contains("txBytes"));
                         QJsonValue txBytes = obj.take("txBytes");
                         Q_ASSERT(txBytes.isDouble());

                         Q_ASSERT(obj.contains("rxBytes"));
                         QJsonValue rxBytes = obj.take("rxBytes");
                         Q_ASSERT(rxBytes.isDouble());

                         emit statusUpdated(serverIpv4Gateway.toString(),
                                            txBytes.toDouble(),
                                            rxBytes.toDouble());
                     });
}

void LinuxController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);

    QDBusPendingCallWatcher *watcher = m_dbus->logs();
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [callback = std::move(callback)](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<QString> reply = *call;
                         if (reply.isError()) {
                             logger.log() << "Error received from the DBus service";
                             callback("Failed to retrieve logs from the mozillavpn-daemon.");
                             return;
                         }

                         QString status = reply.argumentAt<0>();
                         callback(status);
                     });
}
