/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"

#include <sys/capability.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QtDBus/QtDBus>

#include "apptracker.h"
#include "bpfsetmark.h"
#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "platforms/linux/linuxdependencies.h"

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

  m_appTracker = new AppTracker(this);
  connect(m_appTracker, SIGNAL(appLaunched(QString, QString)), this,
          SLOT(appLaunched(QString, QString)));
  connect(m_appTracker, SIGNAL(appTerminated(QString, QString)), this,
          SLOT(appTerminated(QString, QString)));

  m_bpfSetMark = new BpfSetMark(this);

  // Setup to track user login sessions.
  QDBusConnection bus = QDBusConnection::systemBus();
  if (!bus.isConnected()) {
    logger.error() << "System bus is not connected?";
  }
  if (!bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserNew", this,
                   SLOT(userCreated(uint, QDBusObjectPath)))) {
    logger.error() << "Failed to connect to UserNew signal";
  }
  if (!bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserRemoved", this,
                   SLOT(userRemoved(uint, QDBusObjectPath)))) {
    logger.error() << "Failed to connect to UserRemoved signal";
  }

  QDBusMessage listUsersCall = QDBusMessage::createMethodCall(
      DBUS_LOGIN_SERVICE, DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "ListUsers");
  QDBusPendingReply<UserDataList> reply = bus.asyncCall(listUsersCall);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
                   SLOT(userListCompleted(QDBusPendingCallWatcher*)));

  // Drop as many root permissions as we are able.
  dropRootPermissions();
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

void DBusService::setAdaptor(DbusAdaptor* adaptor) {
  Q_ASSERT(!m_adaptor);
  m_adaptor = adaptor;
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

bool DBusService::activate(const QString& jsonConfig) {
  logger.debug() << "Activate";

  if (!isCallerAuthorized()) {
    logger.error() << "Insufficient caller permissions";
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(jsonConfig.toLocal8Bit());
  if (!json.isObject()) {
    logger.error() << "Invalid input";
    return false;
  }

  QJsonObject obj = json.object();

  InterfaceConfig config;
  if (!parseConfig(obj, config)) {
    logger.error() << "Invalid configuration";
    return false;
  }

  if (!Daemon::activate(config)) {
    return false;
  }

  // (Re)load the split tunnelling configuration.
  clearAppStates();
  if (obj.contains("vpnDisabledApps")) {
    QJsonArray disabledApps = obj["vpnDisabledApps"].toArray();
    for (const QJsonValue& app : disabledApps) {
      setAppState(LinuxDependencies::desktopFileId(app.toString()), Excluded);
    }
  }

  return true;
}

bool DBusService::deactivate(bool emitSignals) {
  logger.debug() << "Deactivate";
  if (!isCallerAuthorized()) {
    logger.error() << "Insufficient caller permissions";
    return false;
  }

  m_sessionUid = 0;
  clearAppStates();
  return Daemon::deactivate(emitSignals);
}

QString DBusService::status() {
  return QString(QJsonDocument(getStatus()).toJson(QJsonDocument::Compact));
}

QString DBusService::getLogs() {
  logger.debug() << "Log request";
  if (!isCallerAuthorized()) {
    logger.error() << "Insufficient caller permissions";
    return QString();
  }

  return Daemon::logs();
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
  QDBusInterface iface(DBUS_LOGIN_SERVICE, path.path(), DBUS_LOGIN_USER,
                       QDBusConnection::systemBus());
  if (!iface.isValid()) {
    return;
  }

  // Ensure that systemd has finished creating the user object.
  QVariant state = iface.property("State");
  if (!state.isValid()) {
    logger.error() << "User" << uid << "has invalid user state";
    return;
  }
  logger.debug() << "User" << uid << "state is:" << state.toString();
  if (state.toString() == "opening") {
    // I can't find a signal to hook into, so we are reduced to polling.
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this,
            [this, uid, path]() { userCreated(uid, path); });
    timer->setSingleShot(true);
    timer->start(100);
    return;
  }

  QVariant runtime = iface.property("RuntimePath");
  if (!runtime.isValid()) {
    logger.error() << "User" << uid << "has invalid runtime path";
    return;
  }
  m_appTracker->userCreated(uid, runtime.toString());
}

void DBusService::userRemoved(uint uid, const QDBusObjectPath& path) {
  Q_UNUSED(path);
  m_appTracker->userRemoved(uid);
}

void DBusService::appLaunched(const QString& cgroup,
                              const QString& desktopFileId) {
  logger.debug() << "tracking:" << cgroup << "id:" << desktopFileId;

  AppState state = m_excludedApps.value(desktopFileId, Active);
  if (state == Active) {
    // Nothing to do here.
    return;
  }

  // Apply firewall rules to this control group.
  m_excludedCgroups[cgroup] = state;
  if (state == Excluded) {
    m_bpfSetMark->attachCgroup(cgroup);
    //m_wgutils->excludeCgroup(cgroup);
  }
}

void DBusService::appTerminated(const QString& cgroup,
                                const QString& desktopFileId) {
  logger.debug() << "terminate:" << cgroup << "id:" << desktopFileId;

  // Remove any firewall rules applied to this control group.
  if (m_excludedCgroups.remove(cgroup)) {
    //m_wgutils->resetCgroup(cgroup);
  }
}

void DBusService::setAppState(const QString& desktopFileId, AppState state) {
  logger.debug() << "Setting" << desktopFileId << "to firewall state" << state;

  // When the App is "Active" there is no special manipulation to do.
  if (state == Active) {
    m_excludedApps.remove(desktopFileId);
    for (const QString& cgroup :
         m_appTracker->findByDesktopFileId(desktopFileId)) {
      m_bpfSetMark->detachCgroup(cgroup);
      //m_wgutils->resetCgroup(cgroup);
    }
    return;
  }

  // Otherwise, apply special handling to any matching control groups.
  m_excludedApps[desktopFileId] = state;
  for (const QString& cgroup :
       m_appTracker->findByDesktopFileId(desktopFileId)) {
    if (m_excludedCgroups.contains(cgroup)) {
      m_bpfSetMark->detachCgroup(cgroup);
      //m_wgutils->resetCgroup(cgroup);
    }
    m_excludedCgroups[cgroup] = state;
    if (state == Excluded) {
      m_bpfSetMark->attachCgroup(cgroup);
      // Excluded control groups are given special netfilter rules to direct
      // their traffic outside of the VPN tunnel.
      //m_wgutils->excludeCgroup(cgroup);
    }
  }
}

/* Clear the firewall and return all applications to the active state */
void DBusService::clearAppStates() {
  logger.debug() << "Clearing excluded app list";
  //m_wgutils->resetAllCgroups();
  m_excludedCgroups.clear();
  m_excludedApps.clear();
}

/* Drop root permissions from the daemon. */
void DBusService::dropRootPermissions() {
  logger.debug() << "Dropping root permissions";

  cap_t caps = cap_get_proc();
  if (caps == nullptr) {
    logger.warning() << "Failed to retrieve process capabilities";
    return;
  }
  auto guard = qScopeGuard([&] { cap_free(caps); });

  // Clear the capability set, which effectively makes us an unpriveleged user.
  cap_clear(caps);

  // Acquire CAP_NET_ADMIN, we need it to perform bringup and management
  // of the network interfaces and Wireguard tunnel.
  //
  // Acquire CAP_SETUID, we need it to masquerade as other users on their
  // session busses for application tracking.
  //
  // NOTE: ptrace is a dangerous permission to hold. If it may be safer to
  // relent on the executable check and grant CAP_NET_ADMIN to the client
  // process during installation.
  //
  // Clear all other capabilities, effectively discarding our root permissions.
  cap_value_t newcaps[] = {CAP_NET_ADMIN, CAP_SETUID};
  const int numcaps = sizeof(newcaps) / sizeof(cap_value_t);
  if (cap_set_flag(caps, CAP_EFFECTIVE, numcaps, newcaps, CAP_SET) ||
      cap_set_flag(caps, CAP_PERMITTED, numcaps, newcaps, CAP_SET)) {
    logger.warning() << "Failed to set process capability flags";
    return;
  }
  if (cap_set_proc(caps) != 0) {
    logger.warning() << "Failed to update process capabilities";
    return;
  }
}

/* Checks to see if the caller has sufficient authorization */
bool DBusService::isCallerAuthorized() {
  if (!calledFromDBus()) {
    // If this is not a D-Bus call, it came from the daemon itself.
    return true;
  }
  const QDBusConnectionInterface* iface =
      QDBusConnection::systemBus().interface();

  // If the VPN is active, and we know the UID that turned it on, as a special
  // case we permit that user full access to the D-Bus API in order to manage
  // the connection.
  if (m_sessionUid != 0) {
    const QDBusReply<uint> reply = iface->serviceUid(message().service());
    if (reply.isValid() && m_sessionUid == reply.value()) {
      return true;
    }
  }
  // Otherwise, if this is the activate method, we permit any non-root user to
  // activate the VPN, but we will remember their UID for later authorization
  // checks.
  else if ((message().type() == QDBusMessage::MethodCallMessage) &&
           (message().member() == "activate")) {
    const QDBusReply<uint> reply = iface->serviceUid(message().service());
    const uint senderuid = reply.value();
    if (reply.isValid() && senderuid != 0) {
      m_sessionUid = senderuid;
      return true;
    }
  }
  // In all other cases, the use of this D-Bus API requires the CAP_NET_ADMIN
  // permission, which we can check by examining the PID of the sender. Note
  // that a zero UID (root) is used as a guard value to fall back to this case.

  // Get the PID of the D-Bus message sender.
  const QDBusReply<uint> reply = iface->servicePid(message().service());
  const uint senderpid = reply.value();
  if (!reply.isValid() || (senderpid == 0)) {
    // Could not lookup the sender's PID. Rejected!
    logger.warning() << "Failed to resolve sender PID";
    return false;
  }

  // Get the capabilties of the sender process.
  cap_t caps = cap_get_pid(senderpid);
  if (caps == nullptr) {
    logger.warning() << "Failed to retrieve process capabilities";
    return false;
  }
  auto guard = qScopeGuard([&] { cap_free(caps); });

  // Check if the calling process has CAP_NET_ADMIN.
  cap_flag_value_t flag;
  if (cap_get_flag(caps, CAP_NET_ADMIN, CAP_EFFECTIVE, &flag) != 0) {
    logger.warning() << "Failed to retrieve process cap_net_admin flags";
    return false;
  }
  return (flag == CAP_SET);
}

// Workaround for QTBUG-108822 by manually registering QDBusObjectPath with the
// D-Bus meta-type system, otherwise we are unable to connect to some signals.
#if QT_VERSION < 0x060403
class QtbugRegistrationProxy {
 public:
  QtbugRegistrationProxy() { qDBusRegisterMetaType<QDBusObjectPath>(); }
};

static QtbugRegistrationProxy s_qtbugRegistrationProxy;
#endif
