/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/linux/linuxnotificationhandler.h"
#include "leakdetector.h"
#include "logger.h"

#include <QtDBus/QtDBus>

constexpr const char* DBUS_ITEM = "org.freedesktop.Notifications";
constexpr const char* DBUS_PATH = "/org/freedesktop/Notifications";
constexpr const char* DBUS_INTERFACE = "org.freedesktop.Notifications";

namespace {
Logger logger(LOG_LINUX, "LinuxNotificationHandler");
}

LinuxNotificationHandler::LinuxNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(LinuxNotificationHandler);
}
LinuxNotificationHandler::~LinuxNotificationHandler() {
  MVPN_COUNT_DTOR(LinuxNotificationHandler);
}

void LinuxNotificationHandler::notify(const QString& title,
                                      const QString& message, int timerSec) {
  /*
  Notify via org.freedesktop.Notifications dbus
  Documentation: https://developer.gnome.org/notification-spec/#commands

  Inputs for "Notify" command are:
  - STRING app_name,
  - UINT32 replaces_id,
  - STRING app_icon,
  - STRING summary,
  - STRING body,
  - ARRAY actions,
  - DICT hints,
  - INT32 expire_timeout
  */
  QDBusInterface n(DBUS_ITEM, DBUS_PATH, DBUS_INTERFACE,
                   QDBusConnection::sessionBus());
  if (!n.isValid()) {
    qWarning("Failed to connect to the notification manager via system dbus");
    return;
  }
  const char* appName =
      "Mozilla VPN";        // This doesn't appear to be getting used
  uint32_t replacesId = 0;  // Don't replace.
  const char* appIcon = MVPN_ICON_PATH;
  QStringList actions;
  QMap<QString, QVariant> hints;
  int32_t expireTimeout = timerSec * 1000;
  n.call("Notify", appName, replacesId, appIcon, title, message, actions, hints,
         expireTimeout);
}
