/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidnotificationhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "androidcontroller.h"

namespace {
Logger logger(LOG_ANDROID, "AndroidNotificationHandler");
}

AndroidNotificationHandler::AndroidNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(AndroidNotificationHandler);

  // TODO:
  //  connect(this, &AndroidController::initialized, this,
  //        &AndroidController::applyStrings, Qt::QueuedConnection);
}
AndroidNotificationHandler::~AndroidNotificationHandler() {
  MVPN_COUNT_DTOR(AndroidNotificationHandler);
}

void AndroidNotificationHandler::notify(NotificationHandler::Message type,
                                        const QString& title,
                                        const QString& message, int timerMsec) {
  Q_UNUSED(title);
  Q_UNUSED(type);
  Q_UNUSED(timerMsec);
  
  logger.debug() << "Send notification - " << message;
  //AndroidController::instance()->setNotificationText(title, message,
  //                                                   timerMsec / 1000);
}



/**
void AndroidController::setNotificationText(const QString& title,
                                            const QString& message,
                                            int timerSec) {
  QJsonObject args;
  args["title"] = title;
  args["message"] = message;
  args["sec"] = timerSec;
  QJsonDocument doc(args);
  QAndroidParcel data;
  data.writeData(doc.toJson());
  m_serviceBinder.transact(ACTION_SET_NOTIFICATION_TEXT, data, nullptr);
}



 * Sets fallback Notification text that should be shown in case the VPN
 * switches into the Connected state without the app open
 * e.g via always-on vpn
 
void AndroidController::applyStrings() {
  QJsonObject localisedMessages;
  localisedMessages["productName"] = qtTrId("vpn.main.productName");
  //% "Ready for you to connect"
  //: Refers to the app - which is currently running the background and waiting
  localisedMessages["idleText"] = qtTrId("vpn.android.notification.isIDLE");
  localisedMessages["notification_group_name"] = L18nStrings::instance()->t(
      L18nStrings::AndroidNotificationsGeneralNotifications);

#undef MESSAGE
  QJsonDocument doc(localisedMessages);
  QAndroidParcel data;
  data.writeData(doc.toJson());
  m_serviceBinder.transact(ACTION_SET_NOTIFICATION_FALLBACK, data, nullptr);
}
*/