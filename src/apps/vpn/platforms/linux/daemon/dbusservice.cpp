/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QtDBus/QtDBus>

#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"

#include <sys/capability.h>

namespace {
Logger logger("DBusService");
}

constexpr const char* APP_STATE_ACTIVE = "active";
constexpr const char* APP_STATE_EXCLUDED = "excluded";
constexpr const char* APP_STATE_BLOCKED = "blocked";

constexpr const char* DBUS_LOGIN_SERVICE = "org.freedesktop.login1";
constexpr const char* DBUS_LOGIN_PATH = "/org/freedesktop/login1";
constexpr const char* DBUS_LOGIN_MANAGER = "org.freedesktop.login1.Manager";

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MZ_COUNT_CTOR(DBusService);

  m_daemonExePath = getPidExePath(getpid());

  m_wgutils = new WireguardUtilsLinux(this);

  if (!removeInterfaceIfExists()) {
    qFatal("Interface `%s` exists and cannot be removed. Cannot proceed!",
           WG_INTERFACE);
  }

  m_appTracker = new AppTracker(this);
  connect(m_appTracker, SIGNAL(appLaunched(QString, QString, int)), this,
          SLOT(appLaunched(QString, QString, int)));
  connect(m_appTracker, SIGNAL(appTerminated(QString, QString)), this,
          SLOT(appTerminated(QString, QString)));

  // Setup to track user login sessions.
  QDBusConnection bus = QDBusConnection::systemBus();
  bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserNew", this,
              SLOT(userCreated(uint, QDBusObjectPath)));
  bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserRemoved", this,
              SLOT(userRemoved(uint, QDBusObjectPath)));

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
  if (!checkCapNetAdmin()) {
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
  firewallClear();
  if (obj.contains("vpnDisabledApps")) {
    QJsonArray disabledApps = obj["vpnDisabledApps"].toArray();
    for (const QJsonValue& app : disabledApps) {
      firewallApp(app.toString(), APP_STATE_EXCLUDED);
    }
  }

  return true;
}

bool DBusService::deactivate(bool emitSignals) {
  logger.debug() << "Deactivate";
  firewallClear();
  return Daemon::deactivate(emitSignals);
}

QString DBusService::status() {
  return QString(QJsonDocument(getStatus()).toJson(QJsonDocument::Compact));
}

QString DBusService::getLogs() {
  logger.debug() << "Log request";
  return Daemon::logs();
}

void DBusService::userListCompleted(QDBusPendingCallWatcher* watcher) {
  QDBusPendingReply<UserDataList> reply = *watcher;
  if (reply.isValid()) {
    UserDataList list = reply.value();
    for (const auto& user : list) {
      m_appTracker->userCreated(user.userid, user.path);
    }
  }

  delete watcher;
}

void DBusService::userCreated(uint uid, const QDBusObjectPath& path) {
  m_appTracker->userCreated(uid, path);
}

void DBusService::userRemoved(uint uid, const QDBusObjectPath& path) {
  m_appTracker->userRemoved(uid, path);
}

void DBusService::appLaunched(const QString& cgroup, const QString& appId,
                              int rootpid) {
  logger.debug() << "tracking:" << cgroup << "appId:" << appId
                 << "PID:" << rootpid;

  // HACK: Quick and dirty split tunnelling.
  // TODO: Apply filtering to currently-running apps too.
  if (m_excludedApps.contains(appId)) {
    m_wgutils->excludeCgroup(cgroup);
  }
}

void DBusService::appTerminated(const QString& cgroup, const QString& appId) {
  logger.debug() << "terminate:" << cgroup;

  // HACK: Quick and dirty split tunnelling.
  // TODO: Apply filtering to currently-running apps too.
  if (m_excludedApps.contains(appId)) {
    m_wgutils->resetCgroup(cgroup);
  }
}

/* Get the list of running applications that the firewall knows about. */
QString DBusService::runningApps() {
  QJsonArray result;

  for (auto i = m_appTracker->begin(); i != m_appTracker->end(); i++) {
    const AppData* data = *i;
    QJsonObject appObject;
    QJsonArray pidList;
    appObject.insert("appId", QJsonValue(data->appId));
    appObject.insert("cgroup", QJsonValue(data->cgroup));
    appObject.insert("rootpid", QJsonValue(data->rootpid));

    for (int pid : data->pids()) {
      pidList.append(QJsonValue(pid));
    }

    appObject.insert("pids", pidList);
    result.append(appObject);
  }

  return QJsonDocument(result).toJson(QJsonDocument::Compact);
}

/* Update the firewall for running applications matching the application ID. */
bool DBusService::firewallApp(const QString& appName, const QString& state) {
  logger.debug() << "Setting" << appName << "to firewall state" << state;

  // Update the split tunnelling state for any running apps.
  for (auto i = m_appTracker->begin(); i != m_appTracker->end(); i++) {
    const AppData* data = *i;
    if (data->appId != appName) {
      continue;
    }
    if (state == APP_STATE_EXCLUDED) {
      m_wgutils->excludeCgroup(data->cgroup);
    } else {
      m_wgutils->resetCgroup(data->cgroup);
    }
  }

  // Update the list of apps to exclude from the VPN.
  if (state != APP_STATE_EXCLUDED) {
    m_excludedApps.removeAll(appName);
  } else if (!m_excludedApps.contains(appName)) {
    m_excludedApps.append(appName);
  }
  return true;
}

/* Update the firewall for the application matching the desired PID. */
bool DBusService::firewallPid(int rootpid, const QString& state) {
#if 0
  ProcessGroup* group = m_pidtracker->group(rootpid);
  if (!group) {
    return false;
  }

  group->state = state;
  group->moveToCgroup(getAppStateCgroup(group->state));

  logger.debug() << "Setting" << group->name << "PID:" << rootpid
                 << "to firewall state" << state;
  return true;
#else
  Q_UNUSED(rootpid);
  Q_UNUSED(state);
  return false;
#endif
}

/* Clear the firewall and return all applications to the active state */
bool DBusService::firewallClear() {
  logger.debug() << "Clearing excluded app list";
  m_wgutils->resetAllCgroups();
  m_excludedApps.clear();
  return true;
}

/* Checks to see if this call is being made with CAP_NET_ADMIN */
bool DBusService::checkCapNetAdmin() {
  if (!calledFromDBus()) {
    // That's unexpected, I don't think we should trust this.
    return false;
  }

  // Get the PID of the D-Bus message sender.
  QDBusConnectionInterface *iface = QDBusConnection::systemBus().interface();
  QDBusReply<uint> reply = iface->servicePid(message().service());
  uint senderpid = reply.value();
  if (!reply.isValid() || (senderpid == 0)) {
    // Could not lookup the sender's PID. Rejected!
    logger.warning() << "Failed to resolve sender PID";
    return false;
  }

  // Allow the call if it was made by the same binary.
  if (!m_daemonExePath.isEmpty() &&
      (getPidExePath(senderpid) == m_daemonExePath)) {
    return true;
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

// static
QString DBusService::getPidExePath(uint pid) {
  char exepath[256];
  char pathbuf[PATH_MAX+1];
  snprintf(exepath, sizeof(exepath), "/proc/%u/exe", pid);
  ssize_t len = readlink(exepath, pathbuf, PATH_MAX);
  if ((len <= 0)  ||  (len >= PATH_MAX)) {
    // Either the call to readlink failed, or the result was truncated.
    return QString();
  }
  
  pathbuf[len] = '\0';
  logger.debug() << "Mapped PID" << pid << "to" << pathbuf;
  return QString::fromLocal8Bit(pathbuf, len);
}
