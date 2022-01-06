/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"
#include "constants.h"
#include "leakdetector.h"
#include "l18nstrings.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#if defined(MVPN_IOS)
#  include "platforms/ios/iosnotificationhandler.h"
#elif defined(MVPN_ANDROID)
#  include "platforms/android/androidnotificationhandler.h"
#else

#  if defined(MVPN_LINUX)
#    include "platforms/linux/linuxsystemtraynotificationhandler.h"
#  endif

#  include "systemtraynotificationhandler.h"
#endif

namespace {
Logger logger(LOG_MAIN, "NotificationHandler");

NotificationHandler* s_instance = nullptr;
}  // namespace

// static
NotificationHandler* NotificationHandler::create(QObject* parent) {
#if defined(MVPN_IOS)
  return new IOSNotificationHandler(parent);
#elif defined(MVPN_ANDROID)
  return new AndroidNotificationHandler(parent);
#else

#  if defined(MVPN_LINUX)
  if (LinuxSystemTrayNotificationHandler::requiredCustomImpl()) {
    return new LinuxSystemTrayNotificationHandler(parent);
  }
#  endif

  return new SystemTrayNotificationHandler(parent);
#endif
}

// static
NotificationHandler* NotificationHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

NotificationHandler::NotificationHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(NotificationHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

NotificationHandler::~NotificationHandler() {
  MVPN_COUNT_DTOR(NotificationHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void NotificationHandler::showNotification() {
  logger.debug() << "Show notification";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() != MozillaVPN::StateMain &&
      // The Disconnected notification should be triggerable
      // on StateInitialize, in case the user was connected during a log-out
      // Otherwise existing notifications showing "connected" would update
      !(vpn->state() == MozillaVPN::StateInitialize &&
        vpn->controller()->state() == Controller::StateOff)) {
    return;
  }

  QString title;
  QString message;
  QString countryCode = vpn->currentServer()->exitCountryCode();
  QString localizedCityName = vpn->currentServer()->localizedCityName();
  QString localizedCountryName =
      vpn->serverCountryModel()->localizedCountryName(countryCode);

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      m_connected = true;

      if (m_switching) {
        m_switching = false;

        if (!SettingsHolder::instance()->serverSwitchNotification()) {
          // Dont show notification if it's turned off.
          return;
        }
        if ((m_switchingLocalizedServerCountry == localizedCountryName) &&
            (m_switchingLocalizedServerCity == localizedCityName)) {
          // Don't show notifications unless the exit server changed, see:
          // https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1719
          return;
        }

        //% "VPN Switched Servers"
        title = qtTrId("vpn.systray.statusSwitch.title");
        //% "Switched from %1, %2 to %3, %4"
        //: Shown as message body in a notification. %1 and %3 are countries, %2
        //: and %4 are cities.
        message = qtTrId("vpn.systray.statusSwtich.message")
                      .arg(m_switchingLocalizedServerCountry)
                      .arg(m_switchingLocalizedServerCity)
                      .arg(localizedCountryName)
                      .arg(localizedCityName);
      } else {
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }
        //% "VPN Connected"
        title = qtTrId("vpn.systray.statusConnected.title");
        //% "Connected to %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(localizedCountryName)
                      .arg(localizedCityName);
      }
      break;

    case Controller::StateOff:
      if (m_connected) {
        m_connected = false;
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }

        //% "VPN Disconnected"
        title = qtTrId("vpn.systray.statusDisconnected.title");
        //% "Disconnected from %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is
        //: the city.
        message = qtTrId("vpn.systray.statusDisconnected.message")
                      .arg(localizedCountryName)
                      .arg(localizedCityName);
      }
      break;

    case Controller::StateSwitching:
      m_connected = true;

      m_switching = true;
      m_switchingLocalizedServerCountry = localizedCountryName;
      m_switchingLocalizedServerCity = localizedCityName;
      break;

    default:
      break;
  }

  Q_ASSERT(title.isEmpty() == message.isEmpty());

  if (!title.isEmpty()) {
    notifyInternal(None, title, message, 2000);
  }
}

void NotificationHandler::captivePortalBlockNotificationRequired() {
  logger.debug() << "Captive portal block notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalBlockTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalBlockMessage2);

  notifyInternal(CaptivePortalBlock, title, message,
                 Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::captivePortalUnblockNotificationRequired() {
  logger.debug() << "Captive portal unblock notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockMessage);

  notifyInternal(CaptivePortalUnblock, title, message,
                 Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::unsecuredNetworkNotification(
    const QString& networkName) {
  logger.debug() << "Unsecured network notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsUnsecuredNetworkTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsUnsecuredNetworkMessage)
          .arg(networkName);

  notifyInternal(UnsecuredNetwork, title, message,
                 Constants::UNSECURED_NETWORK_ALERT_MSEC);
}

// TODO: Server unavailable notification
void NotificationHandler::serverUnavailableNotification() {
  logger.debug() << "Server unavailable notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title = l18nStrings->t(L18nStrings::ServerUnavailableModalHeaderText);
  QString message = l18nStrings->t(L18nStrings::ServerUnavailableModalBodyText);

  notifyInternal(ServerUnavailable, title, message,
                 Constants::SERVER_UNAVAILABLE_ALERT_MSEC);
}

void NotificationHandler::notifyInternal(Message type, const QString& title,
                                         const QString& message,
                                         int timerMsec) {
  m_lastMessage = type;

  emit notificationShown(title, message);
  notify(type, title, message, timerMsec);
}

void NotificationHandler::messageClickHandle() {
  logger.debug() << "Message clicked";

  if (m_lastMessage == None) {
    logger.warning() << "Random message clicked received";
    return;
  }

  emit notificationClicked(m_lastMessage);
  m_lastMessage = None;
}
