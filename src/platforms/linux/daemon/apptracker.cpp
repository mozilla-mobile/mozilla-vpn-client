/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apptracker.h"

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

constexpr const char* GTK_DESKTOP_APP_SERVICE = "org.gtk.gio.DesktopAppInfo";
constexpr const char* GTK_DESKTOP_APP_PATH = "/org/gtk/gio/DesktopAppInfo";

constexpr const char* DBUS_SYSTEMD_SERVICE = "org.freedesktop.systemd1";
constexpr const char* DBUS_SYSTEMD_PATH = "/org/freedesktop/systemd1";
constexpr const char* DBUS_SYSTEMD_MANAGER = "org.freedesktop.systemd1.Manager";
constexpr const char* DBUS_SYSTEMD_UNIT = "org.freedesktop.systemd1.Unit";

namespace {
Logger logger("AppTracker");
}  // namespace

AppTracker::AppTracker(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(AppTracker);
  logger.debug() << "AppTracker created.";

  /* Monitor for changes to the user's application control groups. */
  m_cgroupMount = LinuxUtils::findCgroup2Path();
}

AppTracker::~AppTracker() {
  MZ_COUNT_DTOR(AppTracker);
  logger.debug() << "AppTracker destroyed.";

  m_runningCgroups.clear();
}

void AppTracker::userCreated(uint userid, const QString& xdgRuntimePath) {
  logger.debug() << "User created uid:" << userid << "at:" << xdgRuntimePath;

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

  /* Connect to the user's session bus. */
  QString busPath = "unix:path=" + xdgRuntimePath + "/bus";
  logger.debug() << "Connection to" << busPath;
  QDBusConnection connection =
      QDBusConnection::connectToBus(busPath, "user-" + QString::number(userid));

  // Watch the user's control groups for new application scopes.
  m_systemdInterface =
      new QDBusInterface(DBUS_SYSTEMD_SERVICE, DBUS_SYSTEMD_PATH,
                         DBUS_SYSTEMD_MANAGER, connection, this);
  QVariant qv = m_systemdInterface->property("ControlGroup");
  if (!m_cgroupMount.isEmpty() && qv.typeId() == QMetaType::QString) {
    QString userCgroupPath = m_cgroupMount + qv.toString();
    logger.debug() << "Monitoring Control Groups v2 at:" << userCgroupPath;

    connect(&m_cgroupWatcher, SIGNAL(directoryChanged(QString)), this,
            SLOT(cgroupsChanged(QString)));

    m_cgroupWatcher.addPath(userCgroupPath);
    m_cgroupWatcher.addPath(userCgroupPath + "/app.slice");

    cgroupsChanged(userCgroupPath);
    cgroupsChanged(userCgroupPath + "/app.slice");
  }
}

void AppTracker::userRemoved(uint userid) {
  logger.debug() << "User removed uid:" << userid;

  QDBusConnection::disconnectFromBus("user-" + QString::number(userid));
}

// static
// Expand unicode escape sequences.
QString AppTracker::decodeUnicodeEscape(const QString& str) {
  static const QRegularExpression re("(\\\\x[0-9A-Fa-f][0-9A-Fa-f])");

  QString result = str;
  qsizetype offset = 0;
  while (offset < result.length()) {
    // Search for the next unicode escape sequence.
    QRegularExpressionMatch match = re.match(result, offset);
    if (!match.hasMatch()) {
      break;
    }

    bool okay;
    qsizetype start = match.capturedStart(0);
    QChar code = match.captured(0).mid(2).toUShort(&okay, 16);
    if (okay && (code != 0)) {
      // Replace the matched escape sequence with the decoded character.
      result.replace(start, match.capturedLength(0), QString(code));
      offset = start + 1;
    } else {
      // If we failed to decode the character, skip passed the matched string.
      offset = match.capturedEnd(0);
    }
  }

  return result;
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
  if (scopeName.startsWith("app-gnome-") ||
      scopeName.startsWith("app-flatpak-")) {
    // These take the forms:
    //   app-gnome-<desktopFileId>-<pid>.scope
    //   app-flatpak-<desktopFileId>-<pid>.scope
    //
    // Some characters (typically hyphens) in the desktop file ID may be encoded
    // as unicode escape sequences to preseve this formatting.
    QString raw = scopeName.section('-', 2, 2);
    if (!raw.isEmpty()) {
      return QString("%1.desktop").arg(decodeUnicodeEscape(raw));
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

  // Query the systemd unit for its SourcePath property, which is set to the
  // desktop file's full path on KDE
  QDBusReply<QDBusObjectPath> objPath =
      m_systemdInterface->call("GetUnit", scopeName);

  QDBusInterface interface(DBUS_SYSTEMD_SERVICE, objPath.value().path(),
                           DBUS_SYSTEMD_UNIT, m_systemdInterface->connection(),
                           this);
  QString source = interface.property("SourcePath").toString();
  if (!source.isEmpty() && source.endsWith(".desktop")) {
    return LinuxUtils::desktopFileId(source);
  }

  // Otherwise, we don't know the desktop ID for this control group.
  return QString();
}

void AppTracker::cgroupsChanged(const QString& directory) {
  QDir dir(directory);
  QDir mountpoint(m_cgroupMount);
  QFileInfoList newScopes = dir.entryInfoList(
      QStringList{"*.scope", "*@autostart.service"}, QDir::Dirs);
  QStringList oldScopes = m_runningCgroups.keys();

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
      QString desktopFileId = findDesktopFileId(path);
      m_runningCgroups[path] = desktopFileId;

      emit appLaunched(path, desktopFileId);
    }
  }

  // Anything left, if it shares the same root directory, has been removed.
  for (const QString& scope : oldScopes) {
    QFileInfo scopeInfo(m_cgroupMount + scope);
    if (scopeInfo.absolutePath() == directory) {
      logger.debug() << "Control group removed:" << scope;
      Q_ASSERT(m_runningCgroups.contains(scope));
      QString desktopFileId = m_runningCgroups.take(scope);

      emit appTerminated(scope, desktopFileId);
    }
  }
}
