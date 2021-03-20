/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/linux/linuxsystemtrayhandler.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"

#include <QtDBus/QtDBus>

constexpr const char* DBUS_ITEM = "org.freedesktop.Notifications";
constexpr const char* DBUS_PATH = "/org/freedesktop/Notifications";
constexpr const char* DBUS_INTERFACE = "org.freedesktop.Notifications";

namespace {
Logger logger(LOG_LINUX, "LinuxSystemTrayHandler");
}  // namespace

// static
bool LinuxSystemTrayHandler::requiredCustomImpl() {
  // This custom systemTrayHandler implementation is required only on Unity.
  QStringList registeredServices = QDBusConnection::sessionBus()
                                       .interface()
                                       ->registeredServiceNames()
                                       .value();
  return registeredServices.contains("com.canonical.Unity");
}

LinuxSystemTrayHandler::LinuxSystemTrayHandler(QObject* parent)
    : SystemTrayHandler(parent) {
  MVPN_COUNT_CTOR(LinuxSystemTrayHandler);
}

LinuxSystemTrayHandler::~LinuxSystemTrayHandler() {
  MVPN_COUNT_DTOR(LinuxSystemTrayHandler);
}

void LinuxSystemTrayHandler::showNotificationInternal(Message type,
                                                      const QString& title,
                                                      const QString& message,
                                                      int timerMsec) {
  QString actionMessage;
  switch (type) {
    case None:
      return SystemTrayHandler::showNotificationInternal(type, title, message,
                                                         timerMsec);

    case UnsecuredNetwork:
      actionMessage = qtTrId("vpn.toggle.on");
      break;

    case CaptivePortalBlock:
      actionMessage = qtTrId("vpn.toggle.off");
      break;

    case CaptivePortalUnblock:
      actionMessage = qtTrId("vpn.toggle.on");
      break;

    default:
      Q_ASSERT(false);
  }

  m_lastMessage = type;
  emit notificationShown(title, message);

  QDBusInterface n(DBUS_ITEM, DBUS_PATH, DBUS_INTERFACE,
                   QDBusConnection::sessionBus());
  if (!n.isValid()) {
    qWarning("Failed to connect to the notification manager via system dbus");
    return;
  }

  uint32_t replacesId = 0;  // Don't replace.
  const char* appIcon = MVPN_ICON_PATH;
  QStringList actions{"", actionMessage};
  QMap<QString, QVariant> hints;
  n.call("Notify", "", replacesId, appIcon, title, message, actions, hints,
         timerMsec);
}
