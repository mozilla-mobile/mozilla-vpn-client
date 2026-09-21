/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"

#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDBus/QtDBus>

#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "platforms/linux/linuxutils.h"
#include "polkithelper.h"

namespace {
Logger logger("DBusService");
}

constexpr const char* DBUS_LOGIN_SERVICE = "org.freedesktop.login1";
constexpr const char* DBUS_LOGIN_PATH = "/org/freedesktop/login1";
constexpr const char* DBUS_LOGIN_MANAGER = "org.freedesktop.login1.Manager";
constexpr const char* DBUS_LOGIN_USER = "org.freedesktop.login1.User";

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MZ_COUNT_CTOR(DBusService);

  m_wgutils = new WireguardUtilsLinux(this);

  if (!removeInterfaceIfExists()) {
    qFatal("Interface `%s` exists and cannot be removed. Cannot proceed!",
           WG_INTERFACE);
  }

  // Setup to track user login sessions.
  QDBusConnection bus = QDBusConnection::systemBus();
  if (!bus.isConnected()) {
    logger.error() << "System bus is not connected?";
  }
  if (!bus.connect(DBUS_LOGIN_SERVICE, DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER,
                   "UserNew", this, SLOT(userCreated(uint, QDBusObjectPath)))) {
    logger.error() << "Failed to connect to UserNew signal";
  }
  if (!bus.connect(DBUS_LOGIN_SERVICE, DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER,
                   "UserRemoved", this,
                   SLOT(userRemoved(uint, QDBusObjectPath)))) {
    logger.error() << "Failed to connect to UserRemoved signal";
  }

  QDBusMessage listUsersCall = QDBusMessage::createMethodCall(
      DBUS_LOGIN_SERVICE, DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "ListUsers");
  QDBusPendingReply<UserDataList> reply = bus.asyncCall(listUsersCall);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
                   SLOT(userListCompleted(QDBusPendingCallWatcher*)));
}

DBusService::~DBusService() { MZ_COUNT_DTOR(DBusService); }

IPUtils* DBusService::iputils() {
  if (!m_iputils) {
    m_iputils = new IPUtilsLinux(this);
  }
  return m_iputils;
}

DnsUtils* DBusService::dnsutils() {
  if (!m_dnsutils) {
    m_dnsutils = new DnsUtilsLinux(this);
  }
  return m_dnsutils;
}

bool DBusService::removeInterfaceIfExists() {
  if (m_wgutils->interfaceExists()) {
    logger.warning() << "Device already exists. Let's remove it.";
    if (!m_wgutils->deleteInterface()) {
      logger.error() << "Failed to remove the device.";
      return false;
    }
  }
  return true;
}

QString DBusService::version() {
  logger.debug() << "Version request";
  return PROTOCOL_VERSION;
}

bool DBusService::activate(const InterfaceConfig& config) {
  logger.debug() << "Activate";
  if (!isCallerAuthorized("org.mozilla.vpn.activate")) {
    logger.error() << "Insufficient caller permissions";
    return false;
  }

  if (!Daemon::activate(config)) {
    return false;
  }

  // (Re)load the split tunnelling configuration.
  clearAppStates();
  for (const QString& app : config.m_vpnDisabledApps) {
    setAppState(LinuxUtils::desktopFileId(app), Excluded);
  }

  return true;
}

bool DBusService::deactivate(bool emitSignals) {
  logger.debug() << "Deactivate";

  if (!isCallerAuthorized("org.mozilla.vpn.deactivate")) {
    logger.error() << "Insufficient caller permissions";
    return false;
  }

  clearAppStates();
  return Daemon::deactivate(emitSignals);
}

QString DBusService::status() {
  logger.debug() << "Status request";

  if (!isCallerAuthorized("org.mozilla.vpn.activate")) {
    logger.error() << "Insufficient caller permissions";
    return QString();
  }

  return QString(QJsonDocument(getStatus()).toJson(QJsonDocument::Compact));
}

QString DBusService::getLogs() {
  logger.debug() << "Log request";

  if (!isCallerAuthorized("org.mozilla.vpn.activate")) {
    logger.error() << "Insufficient caller permissions";
    return QString();
  }

  return Daemon::logs();
}

void DBusService::cleanupLogs() {
  logger.debug() << "Cleanup logs request";

  if (!isCallerAuthorized("org.mozilla.vpn.activate")) {
    logger.error() << "Insufficient caller permissions";
    return;
  }

  cleanLogs();
}

void DBusService::userListCompleted(QDBusPendingCallWatcher* watcher) {
  QDBusPendingReply<UserDataList> reply = *watcher;
  if (reply.isValid()) {
    UserDataList list = reply.value();
    for (const auto& user : list) {
      userCreated(user.userid, user.path);
    }
  }

  delete watcher;
}

void DBusService::userCreated(uint uid, const QDBusObjectPath& path) {
  Q_UNUSED(uid);

  // Create a new AppTracker instance for this user.
  if (!m_appTrackers.contains(path.path())) {
    AppTracker* tracker = new AppTracker(path.path(), this);
    m_appTrackers.insert(path.path(), tracker);

    connect(tracker, SIGNAL(appLaunched(QString, QString)), this,
            SLOT(appLaunched(QString, QString)));
    connect(tracker, SIGNAL(appTerminated(QString, QString)), this,
            SLOT(appTerminated(QString, QString)));
  }
}

void DBusService::userRemoved(uint uid, const QDBusObjectPath& path) {
  Q_UNUSED(uid);

  AppTracker* tracker = m_appTrackers.take(path.path());
  if (!tracker) {
    return;
  }

  // Drop all control groups from this user.
  for (const QString& cgroup : tracker->appControlGroups()) {
    if (m_excludedCgroups.remove(cgroup)) {
      m_wgutils->resetCgroup(cgroup);
    }
  }

  delete tracker;
}

void DBusService::appLaunched(const QString& cgroup,
                              const QString& desktopFileId) {
  AppTracker* sender = qobject_cast<AppTracker*>(QObject::sender());
  uint uid = sender ? sender->userId() : 0;
  logger.debug() << QString("tracking(%1):").arg(uid)
                 << basename(qPrintable(cgroup)) << "id:" << desktopFileId;

  AppState state = m_excludedApps.value(desktopFileId, Active);
  if (state == Active) {
    // Nothing to do here.
    return;
  }

  // Apply firewall rules to this control group.
  m_excludedCgroups[cgroup] = state;
  if (state == Excluded) {
    m_wgutils->excludeCgroup(cgroup);
  }
}

void DBusService::appTerminated(const QString& cgroup,
                                const QString& desktopFileId) {
  AppTracker* sender = qobject_cast<AppTracker*>(QObject::sender());
  uint uid = sender ? sender->userId() : 0;
  logger.debug() << QString("terminate(%1):").arg(uid)
                 << basename(qPrintable(cgroup)) << "id:" << desktopFileId;

  // Remove any firewall rules applied to this control group.
  if (m_excludedCgroups.remove(cgroup)) {
    m_wgutils->resetCgroup(cgroup);
  }
}

QStringList DBusService::findByDesktopFileId(const QString& id) const {
  QStringList result;
  for (auto i = m_appTrackers.cbegin(); i != m_appTrackers.cend(); i++) {
    result.append(i.value()->findByDesktopFileId(id));
  }
  return result;
}

void DBusService::setAppState(const QString& desktopFileId, AppState state) {
  logger.debug() << "Setting" << desktopFileId << "to firewall state" << state;

  // When the App is "Active" there is no special manipulation to do.
  if (state == Active) {
    m_excludedApps.remove(desktopFileId);
    for (const QString& cgroup : findByDesktopFileId(desktopFileId)) {
      m_wgutils->resetCgroup(cgroup);
    }
    return;
  }

  // Otherwise, apply special handling to any matching control groups.
  m_excludedApps[desktopFileId] = state;
  for (const QString& cgroup : findByDesktopFileId(desktopFileId)) {
    if (m_excludedCgroups.contains(cgroup)) {
      m_wgutils->resetCgroup(cgroup);
    }
    m_excludedCgroups[cgroup] = state;
    if (state == Excluded) {
      // Excluded control groups are given special netfilter rules to direct
      // their traffic outside of the VPN tunnel.
      m_wgutils->excludeCgroup(cgroup);
    }
  }
}

/* Clear the firewall and return all applications to the active state */
void DBusService::clearAppStates() {
  logger.debug() << "Clearing excluded app list";
  m_wgutils->resetAllCgroups();
  m_excludedCgroups.clear();
  m_excludedApps.clear();
}

/* Checks to see if the caller has sufficient authorization */
bool DBusService::isCallerAuthorized(const QString& actionId) {
  if (!calledFromDBus()) {
    // If this is not a D-Bus call, it came from the daemon itself.
    return true;
  }

  if (PolkitHelper::instance()->checkAuthorization(actionId,
                                                   message().service())) {
    logger.debug() << "Polkit authorization granted";
    return true;
  }

  logger.warning() << "Polkit authorization denied";
  return false;
}
