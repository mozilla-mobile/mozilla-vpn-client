/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcontroller.h"

#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QString>

#include "backendlogsobserver.h"
#include "dbusclient.h"
#include "errorhandler.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"

namespace {
Logger logger("LinuxController");
}

LinuxController::LinuxController() {
  MZ_COUNT_CTOR(LinuxController);

  m_dbus = new DBusClient(this);
  connect(m_dbus, &DBusClient::connected, this,
          [this](auto key) { emit connected(key); });
  connect(m_dbus, &DBusClient::disconnected, this,
          &LinuxController::disconnected);

  // Watch for restarts of the D-Bus service.
  m_serviceWatcher = new QDBusServiceWatcher(this);
  m_serviceWatcher->setConnection(QDBusConnection::systemBus());
  m_serviceWatcher->addWatchedService("org.mozilla.vpn.dbus");
  connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
          &LinuxController::dbusNameOwnerChanged);
}

LinuxController::~LinuxController() { MZ_COUNT_DTOR(LinuxController); }

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
    logger.error() << "Error received from the DBus service";
    emit initialized(false, false, QDateTime());
    return;
  }

  QString status = reply.argumentAt<0>();
  logger.debug() << "Status:" << status;

  QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
  Q_ASSERT(json.isObject());

  QJsonObject obj = json.object();
  Q_ASSERT(obj.contains("connected"));
  QJsonValue statusValue = obj.value("connected");
  Q_ASSERT(statusValue.isBool());

  emit initialized(true, statusValue.toBool(), QDateTime::currentDateTime());
}

void LinuxController::dbusNameOwnerChanged(const QString& name,
                                           const QString& prevOwner,
                                           const QString& newOwner) {
  // If the daemon stops, or re-starts then we have been disconnected.
  if (name == m_dbus->serviceName()) {
    logger.info() << "DBus name" << name << "has changed owner to" << newOwner;
    emit disconnected();
  }
}

void LinuxController::activate(const InterfaceConfig& config,
                               Controller::Reason reason) {
  Q_UNUSED(reason);

  connect(m_dbus->activate(config), &QDBusPendingCallWatcher::finished, this,
          &LinuxController::operationCompleted);

  logger.debug() << "LinuxController activated";
}

void LinuxController::deactivate(Controller::Reason reason) {
  logger.debug() << "LinuxController deactivated";

  if (reason == Controller::ReasonSwitching) {
    logger.debug() << "No disconnect for quick server switching";
    emit disconnected();
    return;
  }

  connect(m_dbus->deactivate(), &QDBusPendingCallWatcher::finished, this,
          &LinuxController::operationCompleted);
}

void LinuxController::operationCompleted(QDBusPendingCallWatcher* call) {
  QDBusPendingReply<bool> reply = *call;
  if (reply.isError()) {
    logger.error() << "Error received from the DBus service";
    REPORTERROR(ErrorHandler::ControllerError, "controller");
    emit disconnected();
    return;
  }

  bool status = reply.argumentAt<0>();
  if (status) {
    logger.debug() << "DBus service says: all good.";
    // we will receive the connected/disconnected() signal;
    return;
  }

  logger.error() << "DBus service says: error.";
  REPORTERROR(ErrorHandler::ControllerError, "controller");
  emit disconnected();
}

void LinuxController::checkStatus() {
  logger.debug() << "Check status";

  QDBusPendingCallWatcher* watcher = m_dbus->status();
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &LinuxController::checkStatusCompleted);
}

void LinuxController::checkStatusCompleted(QDBusPendingCallWatcher* call) {
  QDBusPendingReply<QString> reply = *call;
  if (reply.isError()) {
    logger.error() << "Error received from the DBus service";
    return;
  }

  QString status = reply.argumentAt<0>();
  logger.debug() << "Status:" << status;

  QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
  Q_ASSERT(json.isObject());

  QJsonObject obj = json.object();
  Q_ASSERT(obj.contains("connected"));
  QJsonValue statusValue = obj.value("connected");
  Q_ASSERT(statusValue.isBool());

  if (!statusValue.toBool()) {
    logger.error() << "Unable to retrieve the status from the interface.";
    return;
  }

  Q_ASSERT(obj.contains("serverIpv4Gateway"));
  QJsonValue serverIpv4Gateway = obj.value("serverIpv4Gateway");
  Q_ASSERT(serverIpv4Gateway.isString());

  Q_ASSERT(obj.contains("deviceIpv4Address"));
  QJsonValue deviceIpv4Address = obj.value("deviceIpv4Address");
  Q_ASSERT(deviceIpv4Address.isString());

  Q_ASSERT(obj.contains("txBytes"));
  QJsonValue txBytes = obj.value("txBytes");
  Q_ASSERT(txBytes.isDouble());

  Q_ASSERT(obj.contains("rxBytes"));
  QJsonValue rxBytes = obj.value("rxBytes");
  Q_ASSERT(rxBytes.isDouble());

  emit statusUpdated(serverIpv4Gateway.toString(), deviceIpv4Address.toString(),
                     txBytes.toDouble(), rxBytes.toDouble());
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
