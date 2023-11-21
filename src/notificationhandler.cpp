/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"

#include "addons/addonmessage.h"
#include "addons/manager/addonmanager.h"
#include "app.h"
#include "constants.h"
#include "controller.h"
#include "i18nstrings.h"
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

constexpr int NOTIFICATION_TIME_MSEC = 2000;

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

  connect(AddonManager::instance(), &AddonManager::addonCreated, this,
          &NotificationHandler::addonCreated);
}

NotificationHandler::~NotificationHandler() {
  MZ_COUNT_DTOR(NotificationHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void NotificationHandler::showNotification() {
  logger.debug() << "Show notification";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() != App::StateMain &&
      // The Disconnected notification should be triggerable
      // on StateInitialize, in case the user was connected during a log-out
      // Otherwise existing notifications showing "connected" would update
      !(vpn->state() == App::StateInitialize &&
        vpn->connectionManager()->state() == ConnectionManager::StateOff)) {
    return;
  }

  // We want to show notifications about the location in use by the controller,
  // which could be different than MozillaVPN::serverData in the rare case of a
  // server-switch request processed in the meantime.
  QString localizedExitCityName =
      vpn->connectionManager()->currentServer().localizedExitCityName();
  QString localizedCountryName =
      vpn->connectionManager()->currentServer().localizedExitCountryName();

  switch (vpn->connectionManager()->state()) {
    case ConnectionManager::StateOn:
      if (m_switching) {
        m_switching = false;

        if (!SettingsHolder::instance()->serverSwitchNotification()) {
          // Dont show notification if it's turned off.
          return;
        }

        QString localizedPreviousExitCountryName =
            vpn->connectionManager()
                ->currentServer()
                .localizedPreviousExitCountryName();
        QString localizedPreviousExitCityName =
            vpn->connectionManager()
                ->currentServer()
                .localizedPreviousExitCityName();

        if ((localizedPreviousExitCountryName == localizedCountryName) &&
            (localizedPreviousExitCityName == localizedExitCityName)) {
          // Don't show notifications unless the exit server changed, see:
          // https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1719
          return;
        }

        // "VPN Switched Servers"
        notifyInternal(
            None,
            I18nStrings::instance()->t(
                I18nStrings::NotificationsVPNSwitchedServersTitle),
            I18nStrings::instance()
                ->t(I18nStrings::NotificationsVPNSwitchedServersMessage)
                .arg(localizedPreviousExitCityName, localizedExitCityName),
            NOTIFICATION_TIME_MSEC);

        return;
      }

      if (!m_connected) {
        m_connected = true;

        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }

        // "VPN Connected"
        ServerData* serverData = vpn->serverData();

        if (serverData->multihop()) {
          QString localizedEntryCityName = vpn->connectionManager()
                                               ->currentServer()
                                               .localizedEntryCityName();

          QString localizedExitCityName =
              vpn->connectionManager()->currentServer().localizedExitCityName();

          notifyInternal(
              None,
              I18nStrings::instance()->t(
                  I18nStrings::NotificationsVPNConnectedTitle),
              I18nStrings::instance()
                  ->t(I18nStrings::NotificationsVPNMultihopConnectedMessages)
                  .arg(localizedExitCityName, localizedEntryCityName),
              NOTIFICATION_TIME_MSEC);
        } else {
          notifyInternal(None,
                         I18nStrings::instance()->t(
                             I18nStrings::NotificationsVPNConnectedTitle),
                         I18nStrings::instance()
                             ->t(I18nStrings::NotificationsVPNConnectedMessages)
                             .arg(localizedExitCityName),
                         NOTIFICATION_TIME_MSEC);
        }
      }
      return;

    case ConnectionManager::StateOff:
      if (m_connected) {
        m_connected = false;
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }
        // "VPN Disconnected"
        ServerData* serverData = vpn->serverData();
        if (serverData->multihop()) {
          QString localizedEntryCityName = vpn->connectionManager()
                                               ->currentServer()
                                               .localizedEntryCityName();

          QString localizedExitCityName =
              vpn->connectionManager()->currentServer().localizedExitCityName();

          notifyInternal(
              None,
              I18nStrings::instance()->t(
                  I18nStrings::NotificationsVPNDisconnectedTitle),
              I18nStrings::instance()
                  ->t(I18nStrings::NotificationsVPNMultihopDisconnectedMessage)
                  .arg(localizedExitCityName, localizedEntryCityName),
              NOTIFICATION_TIME_MSEC);
        } else {
          notifyInternal(
              None,
              I18nStrings::instance()->t(
                  I18nStrings::NotificationsVPNDisconnectedTitle),
              I18nStrings::instance()
                  ->t(I18nStrings::NotificationsVPNDisconnectedMessage)
                  .arg(localizedExitCityName),
              NOTIFICATION_TIME_MSEC);
        }
      }
      return;

    case ConnectionManager::StateSilentSwitching:
      m_connected = true;
      m_switching = false;
      return;

    case ConnectionManager::StateSwitching:
      m_connected = true;
      m_switching = true;
      return;

    default:
      return;
  }

  Q_ASSERT(false);
}

void NotificationHandler::captivePortalBlockNotificationRequired() {
  logger.debug() << "Captive portal block notification shown";

  I18nStrings* i18nStrings = I18nStrings::instance();
  Q_ASSERT(i18nStrings);

  QString title =
      i18nStrings->t(I18nStrings::NotificationsCaptivePortalBlockTitle);
  QString message =
      i18nStrings->t(I18nStrings::NotificationsCaptivePortalBlockMessage2);

  notifyInternal(CaptivePortalBlock, title, message,
                 Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::captivePortalUnblockNotificationRequired() {
  logger.debug() << "Captive portal unblock notification shown";

  I18nStrings* i18nStrings = I18nStrings::instance();
  Q_ASSERT(i18nStrings);

  QString title =
      i18nStrings->t(I18nStrings::NotificationsCaptivePortalUnblockTitle);
  QString message =
      i18nStrings->t(I18nStrings::NotificationsCaptivePortalUnblockMessage2);

  notifyInternal(CaptivePortalUnblock, title, message,
                 Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::unsecuredNetworkNotification(
    const QString& networkName) {
  logger.debug() << "Unsecured network notification shown";

  I18nStrings* i18nStrings = I18nStrings::instance();
  Q_ASSERT(i18nStrings);

  QString title =
      i18nStrings->t(I18nStrings::NotificationsUnsecuredNetworkTitle);
  QString message =
      i18nStrings->t(I18nStrings::NotificationsUnsecuredNetworkMessage)
          .arg(networkName);

  notifyInternal(UnsecuredNetwork, title, message,
                 Constants::UNSECURED_NETWORK_ALERT_MSEC);
}

void NotificationHandler::serverUnavailableNotification(bool pingRecieved) {
  logger.debug() << "Server unavailable notification shown";

  if (!SettingsHolder::instance()->serverUnavailableNotification()) {
    // Dont show notification if it's turned off.
    return;
  }

  I18nStrings* i18nStrings = I18nStrings::instance();
  Q_ASSERT(i18nStrings);

  QString title = i18nStrings->t(I18nStrings::ServerUnavailableModalHeaderText);
  QString message =
      pingRecieved
          ? i18nStrings->t(
                I18nStrings::
                    ServerUnavailableNotificationBodyTextFireWallBlocked)
          : i18nStrings->t(I18nStrings::ServerUnavailableNotificationBodyText);

  notifyInternal(ServerUnavailable, title, message,
                 Constants::SERVER_UNAVAILABLE_ALERT_MSEC);
}

void NotificationHandler::newInAppMessageNotification(const QString& title,
                                                      const QString& message) {
  logger.debug() << "New in-app message notification";

  if (!App::isUserAuthenticated()) {
    logger.debug() << "User not authenticated, will not be notified.";
    return;
  }

  notifyInternal(NewInAppMessage, title, message,
                 Constants::NEW_IN_APP_MESSAGE_ALERT_MSEC);
}

void NotificationHandler::subscriptionNotFoundNotification() {
  logger.debug() << "Subscription not found notification";

  I18nStrings* i18nStrings = I18nStrings::instance();
  Q_ASSERT(i18nStrings);

  QString notificationTitle =
      i18nStrings->t(I18nStrings::MobileOnboardingPanelOneTitle);
  QString notificationBody =
      i18nStrings->t(I18nStrings::NotificationsSubscriptionNotFound);

  notifyInternal(SubscriptionNotFound, notificationTitle, notificationBody,
                 Constants::DEFAULT_OS_NOTIFICATION_MSEC);
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

void NotificationHandler::addonCreated(Addon* addon) {
  if (addon->type() != "message") {
    return;
  }

  if (addon->enabled() && qobject_cast<AddonMessage*>(addon)->shouldNotify()) {
    maybeAddonNotification(addon);
  }

  connect(addon, &Addon::conditionChanged, this, [this, addon](bool enabled) {
    if (enabled) {
      maybeAddonNotification(addon);
    }
  });
}

void NotificationHandler::maybeAddonNotification(Addon* addon) {
  Q_ASSERT(addon->type() == "message");

  AddonMessage* addonMessage = qobject_cast<AddonMessage*>(addon);
  if (addonMessage->isReceived()) {
    newInAppMessageNotification(addon->property("title").toString(),
                                addon->property("subtitle").toString());
    addonMessage->updateMessageStatus(AddonMessage::MessageStatus::Notified);
  }
}
