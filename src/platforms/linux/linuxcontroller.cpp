/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcontroller.h"
#include "backendlogsobserver.h"
#include "dbusclient.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"
#include "mozillavpn.h"

#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QString>

namespace {
Logger logger({LOG_LINUX, LOG_CONTROLLER}, "LinuxController");
}

LinuxController::LinuxController() {
  MVPN_COUNT_CTOR(LinuxController);

  m_dbus = new DBusClient(this);
  connect(m_dbus, &DBusClient::connected, this, &LinuxController::connected);
  connect(m_dbus, &DBusClient::disconnected, this,
          &LinuxController::disconnected);
}

LinuxController::~LinuxController() { MVPN_COUNT_DTOR(LinuxController); }

void LinuxController::initialize(const Device* device, const Keys* keys) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  QDBusPendingCallWatcher* watcher = m_dbus->status();
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &LinuxController::initializeCompleted);
}

void LinuxController::initializeCompleted(QDBusPendingCallWatcher* call) {
  QDBusPendingReply<QString> reply = *call;
  if (reply.isError()) {
    logger.log() << "Error received from the DBus service";
    emit initialized(false, false, QDateTime());
    return;
  }

  QString status = reply.argumentAt<0>();
  logger.log() << "Status:" << status;

  QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
  Q_ASSERT(json.isObject());

  QJsonObject obj = json.object();
  Q_ASSERT(obj.contains("status"));
  QJsonValue statusValue = obj.value("status");
  Q_ASSERT(statusValue.isBool());

  emit initialized(true, statusValue.toBool(), QDateTime::currentDateTime());
}

void LinuxController::activate(
    const Server& server, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QList<QString>& vpnDisabledApps, Reason reason) {
  Q_UNUSED(reason);
  Q_UNUSED(vpnDisabledApps);

  logger.log() << "LinuxController activated";
  connect(m_dbus->activate(server, device, keys, allowedIPAddressRanges),
          &QDBusPendingCallWatcher::finished, this,
          &LinuxController::operationCompleted);
}

void LinuxController::deactivate(Reason reason) {
  logger.log() << "LinuxController deactivated";

  if (reason == ReasonSwitching) {
    logger.log() << "No disconnect for quick server switching";
    emit disconnected();
    return;
  }

  connect(m_dbus->deactivate(), &QDBusPendingCallWatcher::finished, this,
          &LinuxController::operationCompleted);
}

void LinuxController::operationCompleted(QDBusPendingCallWatcher* call) {
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
}

void LinuxController::checkStatus() {
  logger.log() << "Check status";

  QDBusPendingCallWatcher* watcher = m_dbus->status();
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &LinuxController::checkStatusCompleted);
}

void LinuxController::checkStatusCompleted(QDBusPendingCallWatcher* call) {
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
  QJsonValue statusValue = obj.value("status");
  Q_ASSERT(statusValue.isBool());

  if (!statusValue.toBool()) {
    logger.log() << "Unable to retrieve the status from the interface.";
    return;
  }

  Q_ASSERT(obj.contains("serverIpv4Gateway"));
  QJsonValue serverIpv4Gateway = obj.value("serverIpv4Gateway");
  Q_ASSERT(serverIpv4Gateway.isString());

  Q_ASSERT(obj.contains("txBytes"));
  QJsonValue txBytes = obj.value("txBytes");
  Q_ASSERT(txBytes.isDouble());

  Q_ASSERT(obj.contains("rxBytes"));
  QJsonValue rxBytes = obj.value("rxBytes");
  Q_ASSERT(rxBytes.isDouble());

  emit statusUpdated(serverIpv4Gateway.toString(), txBytes.toDouble(),
                     rxBytes.toDouble());
}

void LinuxController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);

  QDBusPendingCallWatcher* watcher = m_dbus->getLogs();
  connect(watcher, &QDBusPendingCallWatcher::finished,
          new BackendLogsObserver(this, std::move(callback)),
          &BackendLogsObserver::completed);
}

void LinuxController::cleanupBackendLogs() { m_dbus->cleanupLogs(); }
