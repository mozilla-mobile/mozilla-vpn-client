/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "notificationhandler.h"

#include "constants.h"
#include "externalophandler.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "modules/modulevpn.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#if defined(MVPN_IOS)
#  include "platforms/ios/iosnotificationhandler.h"
#endif

#if defined(MVPN_ANDROID)
#  include "platforms/android/androidnotificationhandler.h"
#endif

#if defined(MVPN_LINUX)
#  include "platforms/linux/linuxsystemtraynotificationhandler.h"
#endif

#if defined(MVPN_MACOS)
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

  ModuleHolder::instance()->forEach([handler](const QString&, Module* module) {
    connect(module, &Module::notificationNeeded, handler,
            &NotificationHandler::notify);
  });

  return handler;
}

// static
NotificationHandler* NotificationHandler::createInternal(QObject* parent) {
#if defined(MVPN_IOS)
  return new IOSNotificationHandler(parent);
#endif

#if defined(MVPN_ANDROID)
  return new AndroidNotificationHandler(parent);
#endif

#if defined(MVPN_LINUX)
  if (LinuxSystemTrayNotificationHandler::requiredCustomImpl()) {
    return new LinuxSystemTrayNotificationHandler(parent);
  }
#endif

#if defined(MVPN_MACOS)
  return new MacosSystemTrayNotificationHandler(parent);
#endif

  return new SystemTrayNotificationHandler(parent);
}

// static
NotificationHandler* NotificationHandler::instance() { return s_instance; }

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

void NotificationHandler::captivePortalBlockNotificationRequired() {
  logger.debug() << "Captive portal block notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalBlockTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalBlockMessage2);

  notify(CaptivePortalBlock, title, message,
         Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void NotificationHandler::captivePortalUnblockNotificationRequired() {
  logger.debug() << "Captive portal unblock notification shown";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString title =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockTitle);
  QString message =
      l18nStrings->t(L18nStrings::NotificationsCaptivePortalUnblockMessage2);

  notify(CaptivePortalUnblock, title, message,
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

  notify(UnsecuredNetwork, title, message,
         Constants::UNSECURED_NETWORK_ALERT_MSEC);
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

  notify(ServerUnavailable, title, message,
         Constants::SERVER_UNAVAILABLE_ALERT_MSEC);
}

void NotificationHandler::newInAppMessageNotification(const QString& title,
                                                      const QString& message) {
  logger.debug() << "New in-app message notification";

  if (!MozillaVPN::instance()->isUserAuthenticated()) {
    logger.debug() << "User not authenticated, will not be notified.";
    return;
  }

  notify(NewInAppMessage, title, message,
         Constants::NEW_IN_APP_MESSAGE_ALERT_MSEC);
}

void NotificationHandler::subscriptionNotFoundNotification() {
  logger.debug() << "Subscription not found notification";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QString notificationTitle =
      l18nStrings->t(L18nStrings::MobileOnboardingPanelOneTitle);
  QString notificationBody =
      l18nStrings->t(L18nStrings::NotificationsSubscriptionNotFound);

  notify(SubscriptionNotFound, notificationTitle, notificationBody,
         Constants::DEFAULT_OS_NOTIFICATION_MSEC);
}

void NotificationHandler::notify(Message type, const QString& title,
                                 const QString& message, int timerMsec) {
  m_lastMessage = type;

  emit notificationShown(title, message);
  notifyInternal(type, title, message, timerMsec);
}

void NotificationHandler::messageClickHandle() {
  logger.debug() << "Message clicked";

  if (m_lastMessage == None) {
    logger.warning() << "Random message clicked received";
    return;
  }

  ExternalOpHandler::instance()->request(
      ExternalOpHandler::OpNotificationClicked);

  emit notificationClicked(m_lastMessage);
  m_lastMessage = None;
}
