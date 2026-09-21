/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apptracker.h"

#include <sys/stat.h>
#include <unistd.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QMetaType>
#include <QScopeGuard>
#include <QtDBus/QtDBus>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/linux/dbustypes.h"
#include "platforms/linux/linuxutils.h"
#include "platforms/linux/xdgportal.h"

constexpr const char* DBUS_SYSTEMD_SERVICE = "org.freedesktop.systemd1";
constexpr const char* DBUS_SYSTEMD_PATH = "/org/freedesktop/systemd1";
constexpr const char* DBUS_SYSTEMD_MANAGER = "org.freedesktop.systemd1.Manager";
constexpr const char* DBUS_SYSTEMD_UNIT = "org.freedesktop.systemd1.Unit";

constexpr const char* DBUS_LOGIN_SERVICE = "org.freedesktop.login1";
constexpr const char* DBUS_LOGIN_USER = "org.freedesktop.login1.User";

namespace {
Logger logger("AppTracker");
}  // namespace

AppTracker::AppTracker(const QString& path, QObject* parent)
    : QObject(parent),
      m_connectionName("apptracker-" + path),
      m_userObject(path) {
  MZ_COUNT_CTOR(AppTracker);
  logger.debug() << "AppTracker created:" << path;

  connect(&m_cgroupWatcher, &QFileSystemWatcher::directoryChanged, this,
          &AppTracker::cgroupsChanged);

  /* Monitor for changes to the user's application control groups. */
  m_cgroupMount = LinuxUtils::findCgroup2Path();

  // Fetch the user's runtime properties.
  userFetch();
}

AppTracker::~AppTracker() {
  MZ_COUNT_DTOR(AppTracker);
  logger.debug() << "AppTracker destroyed:" << m_userObject;

  if (m_systemdInterface) {
    QDBusConnection::disconnectFromBus(m_connectionName);
  }

  m_runningCgroups.clear();
}

void AppTracker::userFetch() {
  auto msg = QDBusMessage::createMethodCall(DBUS_LOGIN_SERVICE, m_userObject,
                                            DBUS_PROPERTY_INTERFACE, "GetAll");
  msg << QVariant(DBUS_LOGIN_USER);

  QDBusConnection bus = QDBusConnection::systemBus();
  bus.callWithCallback(msg, this, SLOT(userPropsFinished(const QVariantMap&)),
                       SLOT(dbusErrorOccurred(const QDBusError&)));
}

void AppTracker::userPropsFinished(const QVariantMap& props) {
  if (props.contains("UID")) {
    m_userId = props.value("UID").toUInt();
  }

  QVariant state = props.value("State");
  if (!state.isValid()) {
    logger.error() << "User" << m_userId << "has invalid user state";
    return;
  }
  logger.debug() << "User" << m_userId << "state is:" << state.toString();
  if (state.toString() == "opening") {
    // I can't find a signal to hook into, so we are reduced to polling.
    QTimer::singleShot(100, this, &AppTracker::userFetch);
    return;
  }

  if (!props.contains("RuntimePath")) {
    logger.warning() << "Failed to find XDG runtime path";
    return;
  }

  userCreated(props.value("RuntimePath").toString());
}

void AppTracker::userCreated(const QString& xdgRuntimePath) {
  logger.debug() << "User" << m_userId << "runtime created:" << xdgRuntimePath;

  // Determine the UID of the user runtime.
  struct stat st;
  if (stat(qPrintable(xdgRuntimePath), &st) != 0) {
    logger.warning() << "Failed to stat XDG runtime path:" << strerror(errno);
    return;
  }
  if (st.st_uid == 0) {
    return;
  }

  /* Acquire the effective UID of the user to connect to their session bus. */
  uid_t realuid = getuid();
  auto guard = qScopeGuard([realuid] {
    if (seteuid(realuid) < 0) {
      logger.warning() << "Failed to restore effective UID";
    }
  });
  if (seteuid(st.st_uid) < 0) {
    logger.warning() << "Failed to set effective UID";
    guard.dismiss();
    return;
  }

  /* Connect to the user's session bus. */
  QString busPath = "unix:path=" + xdgRuntimePath + "/bus";
  logger.debug() << "Connection to" << busPath;
  QDBusConnection conn =
      QDBusConnection::connectToBus(busPath, m_connectionName);

  // Watch the user's control groups for new application scopes.
  m_systemdInterface =
      new QDBusInterface(DBUS_SYSTEMD_SERVICE, DBUS_SYSTEMD_PATH,
                         DBUS_SYSTEMD_MANAGER, conn, this);

  // Fetch the user's control group to begin monitoring application scopes.
  auto msg = QDBusMessage::createMethodCall(
      DBUS_SYSTEMD_SERVICE, DBUS_SYSTEMD_PATH, DBUS_PROPERTY_INTERFACE, "Get");
  msg << QVariant(DBUS_SYSTEMD_MANAGER);
  msg << QVariant("ControlGroup");
  conn.callWithCallback(msg, this,
                        SLOT(cgroupPropFinished(const QDBusVariant&)),
                        SLOT(dbusErrorOccurred(const QDBusError&)));
}

void AppTracker::cgroupPropFinished(const QDBusVariant& cgroup) {
  m_userCgroup = cgroup.variant().toString();
  if (m_cgroupMount.isEmpty() || m_userCgroup.isEmpty()) {
    return;
  }

  QDir mountpoint(m_cgroupMount);
  if (m_userCgroup.front() != '/') {
    return;
  }
  if (!mountpoint.exists(m_userCgroup.sliced(1))) {
    return;
  }

  QString userCgroupPath = m_cgroupMount + m_userCgroup;
  logger.debug() << QString("cgroup(%1)").arg(m_userId)
                 << "tracking:" << m_userCgroup;
  m_cgroupWatcher.addPath(userCgroupPath);
  m_cgroupWatcher.addPath(userCgroupPath + "/app.slice");

  cgroupsChanged(userCgroupPath);
  cgroupsChanged(userCgroupPath + "/app.slice");
}

void AppTracker::dbusErrorOccurred(const QDBusError& err) {
  logger.warning() << "dbus error:" << err.message();
}

// The naming convention for snaps follows one of the following formats:
//   snap.<pkg>.<app>.service - assigned by systemd for services
//   snap.<pkg>.<app>-<uuid>.scope - transient scope for apps
//   snap.<pkg>.hook.<app>-<uuid>.scope - transient scope for hooks
//
// However, at some point the separator between the app and UUID was
// swapped from a dot to a dash. Which makes the parsing a bit of a pain.
//
// See: https://github.com/snapcore/snapd/blob/master/sandbox/cgroup/scanning.go
QString AppTracker::snapDesktopFileId(const QString& scope) {
  static const QRegularExpression snapuuid(
      "[-.][0-9a-fA-F]{8}\\b-[0-9a-fA-F]{4}\\b-[0-9a-fA-F]{4}\\b-[0-9a-fA-F]{4}"
      "\\b-[0-9a-fA-F]{12}");

  // Strip the UUID out of the scope name
  QString stripped(scope);
  stripped.remove(snapuuid);

  // Split the remainder on dots and discard the extension.
  QStringList split = stripped.split('.');
  split.removeLast();

  // Parse the package and application.
  QString package = split.value(1);
  QString app = split.value(2);
  if (app == "hook") {
    app = split.value(3);
  }
  if (package.isEmpty() || app.isEmpty()) {
    return QString();
  }

  // Reassemble the desktop identifier.
  return QString("%1_%2.desktop").arg(package).arg(app);
}

// Make an attempt to resolve the desktop ID from a cgroup scope.
QString AppTracker::findDesktopFileId(const QString& cgroup) {
  QString scopeName = QFileInfo(cgroup).fileName();

  // Reverse the desktop ID from a cgroup scope and known launcher tools.
  if (scopeName.startsWith("app-")) {
    QString appId = XdgPortal::parseCgroupAppId(scopeName);
    if (!appId.isEmpty()) {
      return appId + ".desktop";
    }
  }

  QString gnomeLaunchdPrefix("gnome-launched-");
  if (scopeName.startsWith(gnomeLaunchdPrefix)) {
    // These take the form:
    //   gnome-launched-<desktopFileId>-<pid>.scope
    //
    // We have seen this on older Gnome desktop environments (eg: Ubuntu 20.04),
    // and there is no escaping on the desktopFileId, meaning that it might
    // contain embedded hyphens. Therefore, we search for the final hyphen that
    // separates the desktopFileId from the PID.
    qsizetype start = gnomeLaunchdPrefix.length();
    qsizetype end = scopeName.lastIndexOf('-');
    if (end > start) {
      return scopeName.mid(start, end - start);
    }
  }

  // Snaps have their own format.
  if (scopeName.startsWith("snap.")) {
    return snapDesktopFileId(scopeName);
  }

  // Otherwise, query the systemd unit for its SourcePath property, which is set
  // to the desktop file's full path on KDE.
  QDBusReply<QDBusObjectPath> objPath =
      m_systemdInterface->call("GetUnit", scopeName);

  QDBusInterface interface(DBUS_SYSTEMD_SERVICE, objPath.value().path(),
                           DBUS_SYSTEMD_UNIT, m_systemdInterface->connection(),
                           this);
  QString source = interface.property("SourcePath").toString();
  if (!source.isEmpty() && source.endsWith(".desktop")) {
    return LinuxUtils::desktopFileId(source);
  }

  // We don't know the desktop ID for this control group.
  return QString();
}

void AppTracker::cgroupsChanged(const QString& directory) {
  QDir dir(directory);
  QDir mountpoint(m_cgroupMount);
  QStringList oldScopes = m_runningCgroups.keys();

  // The entire directory has been removed.
  if (!dir.exists()) {
    QString cgroup = mountpoint.relativeFilePath(directory);
    if (!cgroup.startsWith('/')) {
      cgroup.prepend('/');
    }
    logger.debug() << QString("cgroup(%1)").arg(m_userId)
                   << "removed:" << cgroup;

    for (const QString& scope : oldScopes) {
      if (!scope.startsWith(cgroup)) {
        continue;
      }

      if ((scope.size() == cgroup.size()) || (scope.at(cgroup.size()) == '/')) {
        QString desktopFileId = m_runningCgroups.take(scope);
        emit appTerminated(scope, desktopFileId);
      }
    }
    return;
  }

  // Figure out what has been added.
  QFileInfoList newScopes = dir.entryInfoList(
      QStringList{"*.scope", "*@autostart.service"}, QDir::Dirs);
  for (const QFileInfo& scope : newScopes) {
    // We need the path starting from the Cgroupv2 mount point.
    QString path = mountpoint.relativeFilePath(scope.canonicalFilePath());
    if (!path.startsWith('/')) {
      path.prepend('/');
    }

    if (oldScopes.removeAll(path) == 0) {
      // This is a new scope, let's add it.
      QString desktopFileId = findDesktopFileId(path);
      m_runningCgroups[path] = desktopFileId;

      emit appLaunched(path, desktopFileId);
    }
  }

  // Anything left, if it shares the same root directory, has been removed.
  for (const QString& scope : oldScopes) {
    QFileInfo scopeInfo(m_cgroupMount + scope);
    if (scopeInfo.absolutePath() == directory) {
      Q_ASSERT(m_runningCgroups.contains(scope));
      QString desktopFileId = m_runningCgroups.take(scope);

      emit appTerminated(scope, desktopFileId);
    }
  }
}
