/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcontroller.h"
#include "backendlogsobserver.h"
#include "dbusclient.h"
#include "errorhandler.h"
#include "ipaddressrange.h"
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
  connect(m_dbus, &DBusClient::connected, this,
          &LinuxController::peerConnected);
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

void LinuxController::activate(
    const QList<Server>& serverList, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QStringList& excludedAddresses, const QStringList& vpnDisabledApps,
    const QHostAddress& dnsServer, Reason reason) {
  Q_UNUSED(reason);
  Q_ASSERT(!serverList.isEmpty());

  // The first hop should exclude the entry server.
  const Server& entry = serverList.last();
  bool first = true;

  // Clear out any connections that might have been lingering.
  m_activationQueue.clear();
  m_device = device;
  m_keys = keys;

  // Activate connections starting from the outermost tunnel
  for (int hopindex = serverList.count() - 1; hopindex > 0; hopindex--) {
    const Server& next = serverList[hopindex - 1];
    HopConnection hop;

    hop.m_server = serverList[hopindex];
    hop.m_hopindex = hopindex;
    hop.m_allowedIPAddressRanges.append(IPAddressRange(next.ipv4AddrIn()));
    hop.m_allowedIPAddressRanges.append(IPAddressRange(next.ipv6AddrIn()));
    if (first) {
      hop.m_excludedAddresses.append(entry.ipv4AddrIn());
      hop.m_excludedAddresses.append(entry.ipv6AddrIn());
      first = false;
    }
    m_activationQueue.append(hop);
  }

  // The final hop should be activated last
  HopConnection lastHop;
  lastHop.m_server = serverList.first();
  lastHop.m_hopindex = 0;
  lastHop.m_allowedIPAddressRanges = allowedIPAddressRanges;
  lastHop.m_excludedAddresses = excludedAddresses;
  if (first) {
    lastHop.m_excludedAddresses.append(entry.ipv4AddrIn());
    lastHop.m_excludedAddresses.append(entry.ipv6AddrIn());
    first = false;
  }
  lastHop.m_vpnDisabledApps = vpnDisabledApps;
  lastHop.m_dnsServer = dnsServer;
  m_activationQueue.append(lastHop);

  logger.debug() << "LinuxController activated";
  activateNext();
}

void LinuxController::activateNext() {
  const HopConnection& hop = m_activationQueue.first();
  connect(
      m_dbus->activate(hop.m_server, m_device, m_keys, hop.m_hopindex,
                       hop.m_allowedIPAddressRanges, hop.m_excludedAddresses,
                       hop.m_vpnDisabledApps, hop.m_dnsServer),
      &QDBusPendingCallWatcher::finished, this,
      &LinuxController::operationCompleted);
}

void LinuxController::deactivate(Reason reason) {
  logger.debug() << "LinuxController deactivated";

  m_activationQueue.clear();

  if (reason == ReasonSwitching) {
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
    MozillaVPN::instance().errorHandle(ErrorHandler::ControllerError);
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
  MozillaVPN::instance().errorHandle(ErrorHandler::ControllerError);
  emit disconnected();
}

// When the daemon reports that a peer connected, activate the next
// connection in the queue, or emit a connected() signal when we are done.
void LinuxController::peerConnected(const QString& pubkey) {
  logger.debug() << "handshake completed with:" << pubkey;
  if (m_activationQueue.isEmpty()) {
    return;
  }

  const HopConnection& hop = m_activationQueue.first();
  if (hop.m_server.publicKey() != pubkey) {
    return;
  }

  m_activationQueue.removeFirst();
  if (m_activationQueue.isEmpty()) {
    emit connected();
  } else {
    activateNext();
  }
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
