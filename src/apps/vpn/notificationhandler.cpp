/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"

#include "appconstants.h"
#include "externalophandler.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#if defined(MZ_IOS)
#  include "platforms/ios/iosnotificationhandler.h"
#endif

#if defined(MZ_ANDROID)
#  include "platforms/android/androidnotificationhandler.h"
#endif

#if defined(MZ_LINUX)
#  include "platforms/linux/linuxsystemtraynotificationhandler.h"
#endif

#if defined(MZ_MACOS)
#  include "platforms/macos/macossystemtraynotificationhandler.h"
#endif

#include "systemtraynotificationhandler.h"

namespace {
Logger logger("NotificationHandler");

NotificationHandler* s_instance = nullptr;
}  // namespace

// static
NotificationHandler* NotificationHandler::create(QObject* parent) {
  NotificationHandler* handler = createInternal(parent);
  handler->initialize();
  return handler;
}

// static
NotificationHandler* NotificationHandler::createInternal(QObject* parent) {
#if defined(MZ_IOS)
  return new IOSNotificationHandler(parent);
#endif

#if defined(MZ_ANDROID)
  return new AndroidNotificationHandler(parent);
#endif

#if defined(MZ_LINUX)
  if (LinuxSystemTrayNotificationHandler::requiredCustomImpl()) {
    return new LinuxSystemTrayNotificationHandler(parent);
  }
#endif

#if defined(MZ_MACOS)
  return new MacosSystemTrayNotificationHandler(parent);
#endif

  return new SystemTrayNotificationHandler(parent);
}

// static
NotificationHandler* NotificationHandler::instance() { return s_instance; }

NotificationHandler::NotificationHandler(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(NotificationHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

NotificationHandler::~NotificationHandler() {
  MZ_COUNT_DTOR(NotificationHandler);

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
  QString localizedCityName = vpn->currentServer()->localizedExitCityName();
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

        QString localizedPreviousExitCountryName =
            vpn->serverCountryModel()->localizedCountryName(
                vpn->currentServer()->previousExitCountryCode());
        QString localizedPreviousExitCityName =
            vpn->currentServer()->localizedPreviousExitCityName();

        if ((localizedPreviousExitCountryName == localizedCountryName) &&
            (localizedPreviousExitCityName == localizedCityName)) {
          // Don't show notifications unless the exit server changed, see:
          // https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1719
          return;
        }
        // "VPN Switched Servers"
        title = L18nStrings::instance()->t(
            L18nStrings::NotificationsVPNSwitchedServersTitle);
        message = L18nStrings::instance()
                      ->t(L18nStrings::NotificationsVPNSwitchedServersMessage)
                      .arg(localizedPreviousExitCityName, localizedCityName);
      } else {
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }
        // "VPN Connected"
        title = L18nStrings::instance()->t(
            L18nStrings::NotificationsVPNConnectedTitle);
        message = L18nStrings::instance()
                      ->t(L18nStrings::NotificationsVPNConnectedMessage)
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
        // "VPN Disconnected"
        title = L18nStrings::instance()->t(
            L18nStrings::NotificationsVPNDisconnectedTitle);
        message = L18nStrings::instance()
                      ->t(L18nStrings::NotificationsVPNDisconnectedMessage)
                      .arg(localizedCityName);
      }
      break;

    case Controller::StateSwitching:
      m_connected = true;
      m_switching = true;
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
                 AppConstants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::captivePortalUnblockNotificationRequired() {
  logger.debug() << "Captive portal unblock notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockMessage2);

  notifyInternal(CaptivePortalUnblock, title, message,
                 AppConstants::CAPTIVE_PORTAL_ALERT_MSEC);
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
                 AppConstants::UNSECURED_NETWORK_ALERT_MSEC);
}

void NotificationHandler::serverUnavailableNotification(bool pingRecieved) {
  logger.debug() << "Server unavailable notification shown";

  if (!SettingsHolder::instance()->serverUnavailableNotification()) {
    // Dont show notification if it's turned off.
    return;
  }

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title = l18nStrings->t(L18nStrings::ServerUnavailableModalHeaderText);
  QString message =
      pingRecieved
          ? l18nStrings->t(
                L18nStrings::
                    ServerUnavailableNotificationBodyTextFireWallBlocked)
          : l18nStrings->t(L18nStrings::ServerUnavailableNotificationBodyText);

  notifyInternal(ServerUnavailable, title, message,
                 AppConstants::SERVER_UNAVAILABLE_ALERT_MSEC);
}

void NotificationHandler::newInAppMessageNotification(const QString& title,
                                                      const QString& message) {
  logger.debug() << "New in-app message notification";

  if (!MozillaVPN::isUserAuthenticated()) {
    logger.debug() << "User not authenticated, will not be notified.";
    return;
  }

  notifyInternal(NewInAppMessage, title, message,
                 AppConstants::NEW_IN_APP_MESSAGE_ALERT_MSEC);
}

void NotificationHandler::subscriptionNotFoundNotification() {
  logger.debug() << "Subscription not found notification";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString notificationTitle =
      l18nStrings->t(L18nStrings::MobileOnboardingPanelOneTitle);
  QString notificationBody =
      l18nStrings->t(L18nStrings::NotificationsSubscriptionNotFound);

  notifyInternal(SubscriptionNotFound, notificationTitle, notificationBody,
                 AppConstants::DEFAULT_OS_NOTIFICATION_MSEC);
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

  if (!ExternalOpHandler::instance()->request(
          ExternalOpHandler::OpNotificationClicked)) {
    return;
  }

  emit notificationClicked(m_lastMessage);
  m_lastMessage = None;
}
