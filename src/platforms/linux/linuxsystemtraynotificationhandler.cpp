/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/linux/linuxsystemtraynotificationhandler.h"

#include <QtDBus/QtDBus>

#include "context/constants.h"
#include "logging/logger.h"
#include "utilities/leakdetector.h"

constexpr const char* DBUS_ITEM = "org.freedesktop.Notifications";
constexpr const char* DBUS_PATH = "/org/freedesktop/Notifications";
constexpr const char* DBUS_INTERFACE = "org.freedesktop.Notifications";
constexpr const char* ACTION_ID = "mozilla_vpn_notification";

namespace {
Logger logger("LinuxSystemTrayNotificationHandler");
}  // namespace

// static
bool LinuxSystemTrayNotificationHandler::requiredCustomImpl() {
  if (!QDBusConnection::sessionBus().isConnected()) {
    return false;
  }

  QDBusConnectionInterface* interface =
      QDBusConnection::sessionBus().interface();
  if (!interface) {
    return false;
  }

  // Use the custom handler on all freedesktop-compliant systems.
  QStringList registeredServices = interface->registeredServiceNames().value();
  return registeredServices.contains(DBUS_ITEM);
}

LinuxSystemTrayNotificationHandler::LinuxSystemTrayNotificationHandler(
    QObject* parent)
    : SystemTrayNotificationHandler(parent) {
  MZ_COUNT_CTOR(LinuxSystemTrayNotificationHandler);
}

LinuxSystemTrayNotificationHandler::~LinuxSystemTrayNotificationHandler() {
  MZ_COUNT_DTOR(LinuxSystemTrayNotificationHandler);
}

void LinuxSystemTrayNotificationHandler::initialize() {
  SystemTrayNotificationHandler::initialize();

  QDBusConnection::sessionBus().connect(DBUS_ITEM, DBUS_PATH, DBUS_INTERFACE,
                                        "ActionInvoked", this,
                                        SLOT(actionInvoked(uint, QString)));
}

void LinuxSystemTrayNotificationHandler::notify(Message type,
                                                const QString& title,
                                                const QString& message,
                                                int timerMsec) {
  QStringList actions;
  switch (type) {
    case None:
    case ServerUnavailable:
    case NewInAppMessage:
      break;

    case UnsecuredNetwork:
      actions.append(ACTION_ID);
      actions.append(qtTrId("vpn.toggle.on"));
      break;

    case CaptivePortalBlock:
      actions.append(ACTION_ID);
      actions.append(qtTrId("vpn.toggle.off"));
      break;

    case CaptivePortalUnblock:
      actions.append(ACTION_ID);
      actions.append(qtTrId("vpn.toggle.on"));
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
  QMap<QString, QVariant> hints;

  QDBusReply<uint> reply =
      n.call("Notify", "Mozilla VPN", replacesId, Constants::LINUX_APP_ID,
             title, message, actions, hints, timerMsec);
  if (!reply.isValid()) {
    logger.warning() << "Failed to show the notification";
  }

  m_lastNotificationId = reply;
}

void LinuxSystemTrayNotificationHandler::actionInvoked(uint actionId,
                                                       QString action) {
  logger.debug() << "Notification clicked" << actionId << action;

  if (action == ACTION_ID && m_lastNotificationId == actionId) {
    messageClickHandle();
  }
}
