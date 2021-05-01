/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QProcessEnvironment>

#include "apptracker.h"
#include "leakdetector.h"
#include "logger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace {
Logger logger(LOG_LINUX, "AppTracker");
}

constexpr const char* GTK_DESKTOP_APP_SERVICE = "org.gtk.gio.DesktopAppInfo";
constexpr const char* GTK_DESKTOP_APP_PATH = "/org/gtk/gio/DesktopAppInfo";

AppTracker::AppTracker(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AppTracker);
  logger.log() << "AppTracker created.";

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (env.contains("DBUS_SESSION_BUS_ADDRESS")) {
    m_busPath = env.value("DBUS_SESSION_BUS_ADDRESS");
  }
  /* If we were launched via sudo, connect to the real user's session. */
  else if (env.contains("SUDO_UID")) {
    m_busPath = "unix:path=/run/user/" + env.value("SUDO_UID") + "/bus";
  } else {
    logger.log() << "Could not locate session D-Bus, consider setting "
                    "DBUS_SESSION_BUS_ADDRESS";
    return;
  }

  /* Assume the effective UID of the bus before connecting, since we starting as
   * root but the D-Bus security policy will prohibit access from different UIDs
   */
  struct stat st;
  uid_t realuid = getuid();
  int err = stat(m_busPath.section('=', 1, 1).toLocal8Bit().data(), &st);
  if (err < 0) {
    logger.log() << "Unable to stat() D-Bus session path:" << strerror(errno);
    return;
  }
  if (realuid != st.st_uid) {
    seteuid(st.st_uid);
  }

  /* Connect to the user's session D-Bus */
  QDBusConnection conn =
      QDBusConnection::connectToBus(m_busPath, "usersession");
  m_interface = new QDBusInterface("", GTK_DESKTOP_APP_PATH,
                                   GTK_DESKTOP_APP_SERVICE, conn, 0);
  if (realuid != st.st_uid) {
    seteuid(realuid);
  }

  bool isConnected = conn.connect(
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
}

void AppTracker::gtkLaunchEvent(const QByteArray& appid, const QString& display,
                                qlonglong pid, const QStringList& uris,
                                const QVariantMap& extra) {
  Q_UNUSED(display);
  Q_UNUSED(uris);
  Q_UNUSED(extra);

  QString appIdName = QString(appid);
  if (!appIdName.isEmpty()) {
    emit appLaunched(appIdName, pid);
  }
}
