/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"
#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "polkithelper.h"

#include <QtDBus/QtDBus>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_LINUX, "DBusService");
}

constexpr const char* APP_STATE_ACTIVE = "active";
constexpr const char* APP_STATE_EXCLUDED = "excluded";
constexpr const char* APP_STATE_BLOCKED = "blocked";

constexpr const char* DBUS_LOGIN_SERVICE = "org.freedesktop.login1";
constexpr const char* DBUS_LOGIN_PATH = "/org/freedesktop/login1";
constexpr const char* DBUS_LOGIN_MANAGER = "org.freedesktop.login1.Manager";

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MVPN_COUNT_CTOR(DBusService);

  m_wgutils = new WireguardUtilsLinux(this);

  if (!removeInterfaceIfExists()) {
    qFatal("Interface `%s` exists and cannot be removed. Cannot proceed!",
           WG_INTERFACE);
  }

  m_appTracker = new AppTracker(this);
  connect(m_appTracker,
          SIGNAL(appLaunched(const QString&, const QString&, int)), this,
          SLOT(appLaunched(const QString&, const QString&, int)));
  connect(m_appTracker, SIGNAL(appTerminated(const QString&, const QString&)),
          this, SLOT(appTerminated(const QString&, const QString&)));

  // Setup to track user login sessions.
  QDBusConnection bus = QDBusConnection::systemBus();
  bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserNew", this,
              SLOT(userCreated(uint, const QDBusObjectPath&)));
  bus.connect("", DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "UserRemoved", this,
              SLOT(userRemoved(uint, const QDBusObjectPath&)));

  QDBusMessage listUsersCall = QDBusMessage::createMethodCall(
      DBUS_LOGIN_SERVICE, DBUS_LOGIN_PATH, DBUS_LOGIN_MANAGER, "ListUsers");
  QDBusPendingReply<UserDataList> reply = bus.asyncCall(listUsersCall);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
                   SLOT(userListCompleted(QDBusPendingCallWatcher*)));
}

DBusService::~DBusService() { MVPN_COUNT_DTOR(DBusService); }

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

  if (!PolkitHelper::instance()->checkAuthorization(
          "org.mozilla.vpn.activate")) {
    logger.error() << "Polkit rejected";
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

  if (obj.contains("vpnDisabledApps")) {
    QJsonArray disabledApps = obj["vpnDisabledApps"].toArray();
    for (const QJsonValue& app : disabledApps) {
      firewallApp(app.toString(), APP_STATE_EXCLUDED);
    }
  }

  return Daemon::activate(config);
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
    for (auto user : list) {
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
