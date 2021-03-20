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
  // Are we on Unity?
  QStringList registeredServices = QDBusConnection::sessionBus()
                                       .interface()
                                       ->registeredServiceNames()
                                       .value();
  m_isUnity = registeredServices.contains("com.canonical.Unity");
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
  QString message =
      qtTrId("vpn.systray.unsecuredNetwork2.message").arg(networkName);
  QString actionMessage = qtTrId("vpn.toggle.on");

  if (m_isUnity) {
    showUnityActionNotification(UnsecuredNetwork, title, actionMessage, message,
                                Constants::UNSECURED_NETWORK_ALERT_MSEC);
  } else {
    SystemTrayHandler::instance()->showNotificationInternal(
        UnsecuredNetwork, title, message,
        Constants::UNSECURED_NETWORK_ALERT_MSEC);
  }
}

void LinuxSystemTrayHandler::captivePortalBlockNotificationRequired() {
  logger.log() << "Captive portal block notification shown";

  //% "Guest Wi-Fi portal blocked"
  QString title = qtTrId("vpn.systray.captivePortalBlock.title");

  //% "The guest Wi-Fi network you’re connected to requires action. Click here"
  //% " to turn off VPN to see the portal."
  QString message = qtTrId("vpn.systray.captivePortalBlock2.message");
  QString actionMessage = qtTrId("vpn.toggle.off");

  if (m_isUnity) {
    showUnityActionNotification(CaptivePortalBlock, title, actionMessage,
                                message, Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  } else {
    SystemTrayHandler::instance()->showNotificationInternal(
        CaptivePortalBlock, title, message,
        Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  }
}

void LinuxSystemTrayHandler::captivePortalUnblockNotificationRequired() {
  logger.log() << "Captive portal unblock notification shown";

  //% "Guest Wi-Fi portal detected"
  QString title = qtTrId("vpn.systray.captivePortalUnblock.title");

  //% "The guest Wi-Fi network you’re connected to may not be secure. Click"
  //% " here to turn on VPN to secure your device."
  QString message = qtTrId("vpn.systray.captivePortalUnblock2.message");
  QString actionMessage = qtTrId("vpn.toggle.on");

  if (m_isUnity) {
    showUnityActionNotification(CaptivePortalUnblock, title, actionMessage,
                                message, Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  } else {
    SystemTrayHandler::instance()->showNotificationInternal(
        CaptivePortalUnblock, title, message,
        Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  }
}

void LinuxSystemTrayHandler::showUnityActionNotification(
    SystemTrayHandler::Message type, const QString& title,
    const QString& actionMessage, const QString& message, int timerMsec) {
  m_lastMessage = type;
  emit notificationShown(title, message);

  QDBusInterface n(DBUS_ITEM, DBUS_PATH, DBUS_INTERFACE,
                   QDBusConnection::sessionBus());
  if (!n.isValid()) {
    qWarning("Failed to connect to the notification manager via system dbus");
    return;
  }

  uint32_t replacesId = 0;  // Don't replace.
  const char* appIcon;
  QStringList actions{"", actionMessage};
  QMap<QString, QVariant> hints;
  n.call("Notify", "", replacesId, appIcon, title, message, actions, hints,
         timerMsec);
}
