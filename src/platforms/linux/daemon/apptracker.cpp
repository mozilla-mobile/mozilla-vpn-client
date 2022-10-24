/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apptracker.h"
#include "dbustypeslinux.h"
#include "leakdetector.h"
#include "logger.h"
#include "../linuxdependencies.h"

#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QScopeGuard>

#include <unistd.h>

constexpr const char* GTK_DESKTOP_APP_SERVICE = "org.gtk.gio.DesktopAppInfo";
constexpr const char* GTK_DESKTOP_APP_PATH = "/org/gtk/gio/DesktopAppInfo";

constexpr const char* DBUS_SYSTEMD_SERVICE = "org.freedesktop.systemd1";
constexpr const char* DBUS_SYSTEMD_PATH = "/org/freedesktop/systemd1";
constexpr const char* DBUS_SYSTEMD_MANAGER = "org.freedesktop.systemd1.Manager";

namespace {
Logger logger(LOG_LINUX, "AppTracker");
QString s_cgroupMount;
}  // namespace

AppTracker::AppTracker(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AppTracker);
  logger.debug() << "AppTracker created.";

  /* Monitor for changes to the user's application control groups. */
  s_cgroupMount = LinuxDependencies::findCgroup2Path();
}

AppTracker::~AppTracker() {
  MVPN_COUNT_DTOR(AppTracker);
  logger.debug() << "AppTracker destroyed.";

  for (AppData* data : m_runningApps) {
    delete data;
  }
  m_runningApps.clear();
}

void AppTracker::userCreated(uint userid, const QDBusObjectPath& path) {
  logger.debug() << "User created uid:" << userid << "at:" << path.path();

  /* Acquire the effective UID of the user to connect to their session bus. */
  uid_t realuid = getuid();
  auto guard = qScopeGuard([&] {
    if (seteuid(realuid) < 0) {
      logger.warning() << "Failed to restore effective UID";
    }
  });
  if (realuid == userid) {
    guard.dismiss();
  } else if (seteuid(userid) < 0) {
    logger.warning() << "Failed to set effective UID";
  }

  /* For correctness we should ask systemd for the user's runtime directory. */
  QString busPath = "unix:path=/run/user/" + QString::number(userid) + "/bus";
  logger.debug() << "Connection to" << busPath;
  QDBusConnection connection =
      QDBusConnection::connectToBus(busPath, "user-" + QString::number(userid));

  /* Connect to the user's GTK launch event. */
  bool gtkConnected = connection.connect(
      "", GTK_DESKTOP_APP_PATH, GTK_DESKTOP_APP_SERVICE, "Launched", this,
      SLOT(gtkLaunchEvent(const QByteArray&, const QString&, qlonglong,
                          const QStringList&, const QVariantMap&)));
  if (!gtkConnected) {
    logger.warning() << "Failed to connect to GTK Launched signal";
  }

  // Watch the user's control groups for new application scopes.
  auto interface = new QDBusInterface(DBUS_SYSTEMD_SERVICE, DBUS_SYSTEMD_PATH,
                                      DBUS_SYSTEMD_MANAGER, connection, this);
  QVariant qv = interface->property("ControlGroup");
  auto dbusguard = qScopeGuard([&] { delete interface; });
  if (!s_cgroupMount.isEmpty() && qv.type() == QVariant::String) {
    QString userCgroupPath = s_cgroupMount + qv.toString();
    logger.debug() << "Monitoring Control Groups v2 at:" << userCgroupPath;

    connect(&m_cgroupWatcher, SIGNAL(directoryChanged(QString)), this,
            SLOT(cgroupsChanged(QString)));

    m_cgroupWatcher.addPath(userCgroupPath);
    m_cgroupWatcher.addPath(userCgroupPath + "/app.slice");

    cgroupsChanged(userCgroupPath);
    cgroupsChanged(userCgroupPath + "/app.slice");
  }
}

void AppTracker::userRemoved(uint userid, const QDBusObjectPath& path) {
  logger.debug() << "User removed uid:" << userid << "at:" << path.path();

  QDBusConnection::disconnectFromBus("user-" + QString::number(userid));
}

void AppTracker::gtkLaunchEvent(const QByteArray& appid, const QString& display,
                                qlonglong pid, const QStringList& uris,
                                const QVariantMap& extra) {
  Q_UNUSED(display);
  Q_UNUSED(uris);
  Q_UNUSED(extra);

  QString appIdName(appid);
  while (appIdName.endsWith('\0')) {
    appIdName.chop(1);
  }
  if (!appIdName.isEmpty()) {
    m_lastLaunchName = appIdName;
    m_lastLaunchPid = pid;
  }
}

void AppTracker::appHeuristicMatch(AppData* data) {
  // If this cgroup contains the last-launched PID, then we have a fairly
  // strong indication of which application this control group is running.
  for (int pid : data->pids()) {
    if ((pid != 0) && (pid == m_lastLaunchPid)) {
      logger.debug() << data->cgroup << "matches app:" << m_lastLaunchName;
      data->appId = m_lastLaunchName;
      data->rootpid = m_lastLaunchPid;
      break;
    }
  }

  // TODO: Some comparison between the .desktop file and the directory name
  // of the control group is also very likely to produce viable application
  // matching, but this will have to be a fuzzy match of some sort because
  // there's a lot of variability in how desktop environments choose to name
  // them.
}

void AppTracker::cgroupsChanged(const QString& directory) {
  QDir dir(directory);
  QDir mountpoint(s_cgroupMount);
  QFileInfoList newScopes =
      dir.entryInfoList(QStringList("*.scope"), QDir::Dirs);
  QStringList oldScopes = m_runningApps.keys();

  // Figure out what has been added.
  for (const QFileInfo& scope : newScopes) {
    // We need the path starting from the Cgroupv2 mount point.
    QString path = mountpoint.relativeFilePath(scope.canonicalFilePath());
    if (!path.startsWith('/')) {
      path.prepend('/');
    }

    if (oldScopes.removeAll(path) == 0) {
      // This is a new scope, let's add it.
      logger.debug() << "Control group created:" << path;
      AppData* data = new AppData(path);

      m_runningApps[path] = data;
      appHeuristicMatch(data);

      emit appLaunched(data->cgroup, data->appId, data->rootpid);
    }
  }

  // Anything left, if it shares the same root directory, has been removed.
  for (const QString& scope : oldScopes) {
    QFileInfo scopeInfo(s_cgroupMount + scope);
    if (scopeInfo.absolutePath() == directory) {
      logger.debug() << "Control group removed:" << scope;
      Q_ASSERT(m_runningApps.contains(scope));
      AppData* data = m_runningApps.take(scope);

      emit appTerminated(data->cgroup, data->appId);
      delete data;
    }
  }
}

QList<int> AppData::pids() const {
  QList<int> results;
  QFile cgroupProcs(s_cgroupMount + cgroup + "/cgroup.procs");

  if (cgroupProcs.open(QIODevice::ReadOnly | QIODevice::Text)) {
    while (true) {
      QString line = QString::fromLocal8Bit(cgroupProcs.readLine());
      if (line.isEmpty()) {
        break;
      }
      int pid = line.trimmed().toInt();
      if (pid != 0) {
        results.append(pid);
      }
    }
    cgroupProcs.close();
  }

  return results;
}
