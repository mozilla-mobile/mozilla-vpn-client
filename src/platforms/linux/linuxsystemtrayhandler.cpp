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
LinuxSystemTrayHandler* s_instance = nullptr;
}  // namespace

// static
LinuxSystemTrayHandler* LinuxSystemTrayHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

LinuxSystemTrayHandler::LinuxSystemTrayHandler(QObject* parent)
    : SystemTrayHandler(parent) {
  MVPN_COUNT_CTOR(LinuxSystemTrayHandler);
  s_instance = this;
}
LinuxSystemTrayHandler::~LinuxSystemTrayHandler() {
  MVPN_COUNT_DTOR(LinuxSystemTrayHandler);
}

void LinuxSystemTrayHandler::unsecuredNetworkNotification(
    const QString& networkName) {
  logger.log() << "Unsecured network notification shown";

  //% "Unsecured Wi-Fi network detected"
  QString title = qtTrId("vpn.systray.unsecuredNetwork.title");

  //% "%1 is not secure. Click here to turn on VPN and secure your device."
  //: %1 is the Wi-Fi network name
  QString actionText =
      qtTrId("vpn.systray.unsecuredNetwork2.message").arg(networkName);

  showActionNotification(UnsecuredNetwork, title, actionText,
                         Constants::UNSECURED_NETWORK_ALERT_MSEC);
}

void LinuxSystemTrayHandler::showMessage(const QStringList actions,
                                         const QString& summary,
                                         const QString& body, int timerMsec) {
  logger.log() << "--------------------------------------- IT'S A DBUS MESSAGE";
  /*
  Notify via org.freedesktop.SystemTrays dbus
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
  QMap<QString, QVariant> hints;
  n.call("Notify", appName, replacesId, appIcon, summary, body, actions, hints,
         timerMsec);
}

void LinuxSystemTrayHandler::showActionNotification(
    SystemTrayHandler::Message type, const QString& title,
    const QString& actionText, int timerMsec) {
  m_lastMessage = type;
  emit notificationShown(title, actionText);
  QStringList actions{"", actionText};
  showMessage(actions, title, "", timerMsec);
}

void LinuxSystemTrayHandler::showNotificationInternal(
    SystemTrayHandler::Message type, const QString& title,
    const QString& message, int timerMsec) {
  m_lastMessage = type;
  emit notificationShown(title, message);
  showMessage(QStringList(), title, message, timerMsec);
}