/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QJsonDocument>
#include <QJsonObject>

#include "firewallservice.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/linux/linuxdependencies.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

constexpr const char* CGROUP_PROCS_FILE = "/cgroup.procs";
constexpr const char* CGROUP_CLASS_FILE = "/net_cls.classid";
constexpr const char* VPN_EXCLUDE_CGROUP = "mozvpn.exclude";
constexpr unsigned long VPN_EXCLUDE_CLASS_ID = 0x00110011;

constexpr const char* LOGIN_MANAGER_SERVICE = "org.freedesktop.login1";
constexpr const char* LOGIN_MANAGER_PATH = "/org/freedesktop/login1";
constexpr const char* LOGIN_MANAGER_INTERFACE =
    "org.freedesktop.login1.Manager";

namespace {
Logger logger(LOG_LINUX, "FirewallService");
}

class UserData {
 public:
  QString name;
  uint userid;
  QDBusObjectPath path;
};
QDBusArgument& operator<<(QDBusArgument& args, const UserData& data) {
  args.beginStructure();
  args << data.userid << data.name << data.path;
  args.endStructure();
  return args;
}
const QDBusArgument& operator>>(const QDBusArgument& args, UserData& data) {
  args.beginStructure();
  args >> data.userid >> data.name >> data.path;
  args.endStructure();
  return args;
}
Q_DECLARE_METATYPE(UserData);
typedef QList<UserData> UserDataList;
Q_DECLARE_METATYPE(UserDataList);

FirewallService::FirewallService(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(FirewallService);
  m_adaptor = new FirewallAdaptor(this);

  qRegisterMetaType<UserData>();
  qDBusRegisterMetaType<UserData>();
  qRegisterMetaType<UserDataList>();
  qDBusRegisterMetaType<UserDataList>();

  /* Resolve the control group paths we need. */
  m_defaultCgroup = LinuxDependencies::findCgroupPath("net_cls");
  m_excludeCgroup = m_defaultCgroup + "/" + VPN_EXCLUDE_CGROUP;

  int err = mkdir(m_excludeCgroup.toLocal8Bit().constData(),
                  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  if ((err < 0) && (errno != EEXIST)) {
    logger.log() << "Failed to create VPN exclusion cgroup:" << strerror(errno);
    return;
  }
  writeCgroupFile(m_excludeCgroup + CGROUP_CLASS_FILE, VPN_EXCLUDE_CLASS_ID);

  m_pidtracker = new PidTracker(this);
  connect(m_pidtracker, SIGNAL(pidForked(const QString&, int, int)), this,
          SLOT(pidForked(const QString&, int, int)));
  connect(m_pidtracker, SIGNAL(terminated(const QString&, int)), this,
          SLOT(appTerminated(const QString&, int)));

  QDBusConnection m_conn = QDBusConnection::systemBus();
  m_conn.connect("", LOGIN_MANAGER_PATH, LOGIN_MANAGER_INTERFACE, "UserNew",
                 this, SLOT(userCreated(uint, const QDBusObjectPath&)));
  m_conn.connect("", LOGIN_MANAGER_PATH, LOGIN_MANAGER_INTERFACE, "UserRemoved",
                 this, SLOT(userRemoved(uint, const QDBusObjectPath&)));

  QDBusInterface n(LOGIN_MANAGER_SERVICE, LOGIN_MANAGER_PATH,
                   LOGIN_MANAGER_INTERFACE, m_conn);
  QDBusPendingReply<UserDataList> reply = n.asyncCall("ListUsers");
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                   &FirewallService::userListCompleted);
}

FirewallService::~FirewallService() { MVPN_COUNT_DTOR(FirewallService); }

QString FirewallService::version() {
  logger.log() << "Version request";
  return PROTOCOL_VERSION;
}

QString FirewallService::status() {
  logger.log() << "Status request";
  QJsonObject json;
  json.insert("status", QJsonValue(true));
  return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QString FirewallService::runningApps() {
  QJsonArray result;

  for (auto app = m_pidtracker->m_processGroups.begin();
       app != m_pidtracker->m_processGroups.end(); app++) {
    const ProcessGroup* group = *app;
    QJsonObject appObject;
    QJsonArray pidList;
    appObject.insert("name", QJsonValue(group->name));
    appObject.insert("userid", QJsonValue((qint64)group->userid));
    appObject.insert("rootpid", QJsonValue(group->rootpid));

    for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
      if (m_pidtracker->group(*pid) == group) {
        pidList.append(QJsonValue(*pid));
      }
    }
    appObject.insert("pids", pidList);
    result.append(appObject);
  }

  return QJsonDocument(result).toJson(QJsonDocument::Compact);
}

bool FirewallService::excludeApp(const QStringList& names) {
  for (auto app = names.begin(); app != names.end(); app++) {
    logger.log() << "Adding" << *app << "to VPN exclusion group";
    if (m_excludedApps.contains(*app)) continue;
    m_excludedApps.append(*app);

    for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
      const ProcessGroup* group = m_pidtracker->group(*pid);
      if (group->name == *app) {
        writeCgroupFile(m_excludeCgroup + CGROUP_PROCS_FILE, *pid);
      }
    }
  }
  return true;
}

bool FirewallService::includeApp(const QStringList& names) {
  for (auto app = names.begin(); app != names.end(); app++) {
    logger.log() << "Removing" << *app << "from VPN exclusion group";
    m_excludedApps.removeAll(*app);

    for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
      const ProcessGroup* group = m_pidtracker->group(*pid);
      if (group->name == *app) {
        writeCgroupFile(m_defaultCgroup + CGROUP_PROCS_FILE, *pid);
      }
    }
  }
  return true;
}

bool FirewallService::flushApps() {
  for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
    const ProcessGroup* group = m_pidtracker->group(*pid);
    if (m_excludedApps.contains(group->name)) {
      writeCgroupFile(m_defaultCgroup + CGROUP_PROCS_FILE, *pid);
    }
  }

  for (auto app = m_excludedApps.begin(); app != m_excludedApps.end(); app++) {
    logger.log() << "Removing" << *app << "from VPN exclusion group";
  }
  m_excludedApps.clear();
  return true;
}

void FirewallService::pidForked(const QString& name, int parent, int child) {
  if (m_excludedApps.contains(name)) {
    writeCgroupFile(m_excludeCgroup + CGROUP_PROCS_FILE, child);
  }
  Q_UNUSED(parent);
}

void FirewallService::appTerminated(const QString& name, int rootpid) {
  logger.log() << "terminate:" << name << "PID:" << rootpid;
}

void FirewallService::appLaunched(const QString& name, uint userid,
                                  int rootpid) {
  if (m_excludedApps.contains(name)) {
    writeCgroupFile(m_excludeCgroup + CGROUP_PROCS_FILE, rootpid);
  }
  logger.log() << "tracking:" << name << "PID:" << rootpid;
  m_pidtracker->track(name, userid, rootpid);
}

void FirewallService::userListCompleted(QDBusPendingCallWatcher* watcher) {
  QDBusPendingReply<UserDataList> reply = *watcher;
  if (!reply.isValid()) {
    return;
  }

  UserDataList list = reply.value();
  for (auto user : list) {
    userCreated(user.userid, user.path);
  }

  delete watcher;
}

void FirewallService::userCreated(uint uid, const QDBusObjectPath& path) {
  if (m_users.contains(uid)) {
    return;
  }
  AppTracker* session = new AppTracker(uid, path, this);
  m_users[uid] = session;
  QObject::connect(session, SIGNAL(appLaunched(const QString&, uint, int)),
                   this, SLOT(appLaunched(const QString&, uint, int)));

  logger.log() << "User created uid:" << uid << "at:" << path.path();
}

void FirewallService::userRemoved(uint uid, const QDBusObjectPath& path) {
  AppTracker* session = m_users.value(uid);
  if (!session) {
    return;
  }
  logger.log() << "User removed uid:" << uid << "at:" << path.path();
  m_users.remove(uid);
  delete session;
}

void FirewallService::writeCgroupFile(const QString& path,
                                      unsigned long value) {
  FILE* fp = fopen(path.toLocal8Bit().constData(), "w");
  if (fp) {
    fprintf(fp, "%lu", value);
  }
  fclose(fp);
}
