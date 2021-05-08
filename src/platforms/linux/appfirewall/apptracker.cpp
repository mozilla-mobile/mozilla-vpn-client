/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apptracker.h"
#include "leakdetector.h"
#include "logger.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QProcessEnvironment>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

constexpr const char* GTK_DESKTOP_APP_SERVICE = "org.gtk.gio.DesktopAppInfo";
constexpr const char* GTK_DESKTOP_APP_PATH = "/org/gtk/gio/DesktopAppInfo";

namespace {
Logger logger(LOG_LINUX, "AppTracker");
}

AppTracker::AppTracker(uint userid, QDBusObjectPath path, QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(AppTracker);
  logger.log() << "AppTracker created.";
  Q_UNUSED(path);

  m_userid = userid;

  /* For correctness should ask systemd for the user's runtime directory. */
  QString busPath = "unix:path=/run/user/" + QString::number(userid) + "/bus";
  logger.log() << "Connection to" << busPath;

  /* The D-Bus security policy will require us to adopt the effective UID of
   * the session bus before we can connect to it. But the firewall needs to
   * be running as root to manage control groups.
   */
  uid_t realuid = getuid();
  if (seteuid(userid) < 0) {
    logger.log() << "Failed to set effective UID";
  }
  QDBusConnection connection =
      QDBusConnection::connectToBus(busPath, "user-" + QString::number(userid));
  if (seteuid(realuid) < 0) {
    logger.log() << "Failed to restore effective UID";
  }

  bool isConnected = connection.connect(
      "", GTK_DESKTOP_APP_PATH, GTK_DESKTOP_APP_SERVICE, "Launched", this,
      SLOT(gtkLaunchEvent(const QByteArray&, const QString&, qlonglong,
                          const QStringList&, const QVariantMap&)));
  if (!isConnected) {
    logger.log() << "Failed to connect to GTK Launched signal";
  }
}

AppTracker::~AppTracker() {
  MVPN_COUNT_DTOR(AppTracker);
  logger.log() << "AppTracker destroyed.";
  QDBusConnection::disconnectFromBus("user-" + QString::number(m_userid));
}

void AppTracker::gtkLaunchEvent(const QByteArray& appid, const QString& display,
                                qlonglong pid, const QStringList& uris,
                                const QVariantMap& extra) {
  Q_UNUSED(display);
  Q_UNUSED(uris);
  Q_UNUSED(extra);

  QString appIdName = QString(appid);
  if (!appIdName.isEmpty()) {
    emit appLaunched(appIdName, m_userid, pid);
  }
}
