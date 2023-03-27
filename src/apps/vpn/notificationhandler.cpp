/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"

#include "addons/addonmessage.h"
#include "addons/manager/addonmanager.h"
#include "app.h"
#include "appconstants.h"
#include "controller.h"
#include "externalophandler.h"
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
        vpn->controller()->state() == Controller::StateOff)) {
    return;
  }

  // We want to show notifications about the location in use by the controller,
  // which could be different than MozillaVPN::serverData in the rare case of a
  // server-switch request processed in the meantime.
  QString localizedCityName =
      vpn->controller()->currentServer().localizedExitCityName();
  QString localizedCountryName =
      vpn->controller()->currentServer().localizedExitCountryName();

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      if (m_switching) {
        m_switching = false;

        if (!SettingsHolder::instance()->serverSwitchNotification()) {
          // Dont show notification if it's turned off.
          return;
        }

        QString localizedPreviousExitCountryName =
            vpn->controller()
                ->currentServer()
                .localizedPreviousExitCountryName();
        QString localizedPreviousExitCityName =
            vpn->controller()->currentServer().localizedPreviousExitCityName();

        if ((localizedPreviousExitCountryName == localizedCountryName) &&
            (localizedPreviousExitCityName == localizedCityName)) {
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
                .arg(localizedPreviousExitCityName, localizedCityName),
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
          QString localizedEntryCityName =
              vpn->controller()->currentServer().localizedEntryCityName();

          QString localizedExitCityName =
              vpn->controller()->currentServer().localizedExitCityName();

          notifyInternal(
              None,
              I18nStrings::instance()->t(
                  I18nStrings::NotificationsVPNConnectedTitle),
              I18nStrings::instance()
                  ->t(I18nStrings::NotificationsVPNMultihopConnectedMessage)
                  .arg(localizedExitCityName, localizedEntryCityName),
              NOTIFICATION_TIME_MSEC);
        } else {
          notifyInternal(None,
                         I18nStrings::instance()->t(
                             I18nStrings::NotificationsVPNConnectedTitle),
                         I18nStrings::instance()
                             ->t(I18nStrings::NotificationsVPNConnectedMessage)
                             .arg(localizedCityName),
                         NOTIFICATION_TIME_MSEC);
        }
      }
      return;

    case Controller::StateOff:
      if (m_connected) {
        m_connected = false;
        if (!SettingsHolder::instance()->connectionChangeNotification()) {
          // Notifications for ConnectionChange are disabled
          return;
        }
        // "VPN Disconnected"
        notifyInternal(None,
                       I18nStrings::instance()->t(
                           I18nStrings::NotificationsVPNDisconnectedTitle),
                       I18nStrings::instance()
                           ->t(I18nStrings::NotificationsVPNDisconnectedMessage)
                           .arg(localizedCityName),
                       NOTIFICATION_TIME_MSEC);
      }
      return;

    case Controller::StateSilentSwitching:
      m_connected = true;
      m_switching = false;
      return;

    case Controller::StateSwitching:
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
                 AppConstants::CAPTIVE_PORTAL_ALERT_MSEC);
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
                 AppConstants::CAPTIVE_PORTAL_ALERT_MSEC);
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
                 AppConstants::UNSECURED_NETWORK_ALERT_MSEC);
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
                 AppConstants::SERVER_UNAVAILABLE_ALERT_MSEC);
}

void NotificationHandler::newInAppMessageNotification(const QString& title,
                                                      const QString& message) {
  logger.debug() << "New in-app message notification";

  if (!App::isUserAuthenticated()) {
    logger.debug() << "User not authenticated, will not be notified.";
    return;
  }

  notifyInternal(NewInAppMessage, title, message,
                 AppConstants::NEW_IN_APP_MESSAGE_ALERT_MSEC);
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
          MozillaVPN::OpNotificationClicked)) {
    return;
  }

  emit notificationClicked(m_lastMessage);
  m_lastMessage = None;
}

void NotificationHandler::addonCreated(Addon* addon) {
  if (addon->type() != "message") {
    return;
  }

  if (addon->enabled()) {
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
