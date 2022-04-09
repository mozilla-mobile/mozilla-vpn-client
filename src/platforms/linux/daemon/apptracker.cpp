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
}

AppTracker::AppTracker(uint userid, const QDBusObjectPath& path,
                       QObject* parent)
    : QObject(parent), m_userid(userid), m_objectPath(path) {
  MVPN_COUNT_CTOR(AppTracker);
  logger.debug() << "AppTracker(" + QString::number(m_userid) + ") created.";

  /* Acquire the effective UID of the user to connect to their session bus. */
  uid_t realuid = getuid();
  auto guard = qScopeGuard([&] {
    if (seteuid(realuid) < 0) {
      logger.warning() << "Failed to restore effective UID";
    }
  });
  if (realuid == m_userid) {
    guard.dismiss();
  } else if (seteuid(m_userid) < 0) {
    logger.warning() << "Failed to set effective UID";
  }

  /* For correctness we should ask systemd for the user's runtime directory. */
  QString busPath = "unix:path=/run/user/" + QString::number(m_userid) + "/bus";
  logger.debug() << "Connection to" << busPath;
  QDBusConnection connection = QDBusConnection::connectToBus(
      busPath, "user-" + QString::number(m_userid));

  /* Connect to the user's GTK launch event. */
  bool gtkConnected = connection.connect(
      "", GTK_DESKTOP_APP_PATH, GTK_DESKTOP_APP_SERVICE, "Launched", this,
      SLOT(gtkLaunchEvent(const QByteArray&, const QString&, qlonglong,
                          const QStringList&, const QVariantMap&)));
  if (!gtkConnected) {
    logger.warning() << "Failed to connect to GTK Launched signal";
  }

  /* Monitor for changes to the user's application control groups. */
  m_interface = new QDBusInterface(DBUS_SYSTEMD_SERVICE, DBUS_SYSTEMD_PATH,
                                   DBUS_SYSTEMD_MANAGER, connection, this);
  QVariant qv = m_interface->property("ControlGroup");
  if (qv.type() == QVariant::String) {
    m_cgroupPath = LinuxDependencies::findCgroup2Path() + qv.toString();
    logger.debug() << "Found Control Groups v2 at:" << m_cgroupPath;

    connect(&m_cgroupWatcher, SIGNAL(directoryChanged(const QString&)), this,
            SLOT(cgroupsChanged(const QString&)));

    m_cgroupWatcher.addPath(m_cgroupPath);
    m_cgroupWatcher.addPath(m_cgroupPath + "/app.slice");

    cgroupsChanged(m_cgroupPath);
    cgroupsChanged(m_cgroupPath + "/app.slice");
  }
}

AppTracker::~AppTracker() {
  MVPN_COUNT_DTOR(AppTracker);
  logger.debug() << "AppTracker(" + QString::number(m_userid) + ") destroyed.";

  QDBusConnection::disconnectFromBus("user-" + QString::number(m_userid));
}

void AppTracker::gtkLaunchEvent(const QByteArray& appid, const QString& display,
                                qlonglong pid, const QStringList& uris,
                                const QVariantMap& extra) {
  Q_UNUSED(display);
  Q_UNUSED(uris);
  Q_UNUSED(extra);

  QString appIdName(appid);
  if (!appIdName.isEmpty()) {
    emit appLaunched(appIdName, pid);
  }
}

void AppTracker::cgroupsChanged(const QString& directory) {
  QDir dir(directory);
  QFileInfoList newScopes =
      dir.entryInfoList(QStringList("*.scope"), QDir::Dirs);
  QStringList oldScopes = m_cgroupScopes;

  // Figure out what has been added.
  for (const QFileInfo& scope : newScopes) {
    QString path = scope.canonicalFilePath();
    if (oldScopes.removeAll(path) == 0) {
      // This is a new scope, let's add it.
      logger.debug() << "Control group created:" << path;
      m_cgroupScopes.append(path);
    }
  }

  // Anything left, if it shares the same root directory, has been removed.
  for (const QString& scope : oldScopes) {
    QFileInfo scopeInfo(scope);
    if (scopeInfo.absolutePath() == directory) {
      logger.debug() << "Control group removed:" << scope;
      m_cgroupScopes.removeAll(scope);
    }
  }
}
