/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QProcessEnvironment>

#include "apptracker.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_LINUX, "AppTracker");
}

constexpr const char* GTK_DESKTOP_APP_SERVICE = "org.gtk.gio.DesktopAppInfo";
constexpr const char* GTK_DESKTOP_APP_PATH = "/org/gtk/gio/DesktopAppInfo";

AppTracker::AppTracker(QDBusConnection connection, QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(AppTracker);
  logger.log() << "AppTracker created.";

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
