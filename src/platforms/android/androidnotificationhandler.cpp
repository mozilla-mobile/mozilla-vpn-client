/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidnotificationhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "androidvpnactivity.h"
#include "l18nstrings.h"

#include <QJsonObject>
#include <QJsonDocument>

namespace {
Logger logger(LOG_ANDROID, "AndroidNotificationHandler");
}

AndroidNotificationHandler::AndroidNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(AndroidNotificationHandler);

  connect(AndroidVPNActivity::instance(), &AndroidVPNActivity::serviceConnected,
          this, &AndroidNotificationHandler::applyStrings,
          Qt::QueuedConnection);
}
AndroidNotificationHandler::~AndroidNotificationHandler() {
  MVPN_COUNT_DTOR(AndroidNotificationHandler);
}

void AndroidNotificationHandler::notify(NotificationHandler::Message type,
                                        const QString& title,
                                        const QString& message, int timerMsec) {
  
  logger.debug() << "Send notification - " << message;
  QJsonObject args;
  args["title"] = title;
  args["message"] = message;
  args["sec"] = timerMsec;
  args["type"] = type;
  QJsonDocument doc(args);
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_SET_NOTIFICATION_TEXT,
                                    doc.toJson());
}

void AndroidNotificationHandler::applyStrings() {
  QJsonObject localisedMessages;
  localisedMessages["productName"] = qtTrId("vpn.main.productName");
  //% "Ready for you to connect"
  //: Refers to the app - which is currently running the background and waiting
  localisedMessages["idleText"] = qtTrId("vpn.android.notification.isIDLE");
  localisedMessages["notification_group_name"] = L18nStrings::instance()->t(
      L18nStrings::AndroidNotificationsGeneralNotifications);

  QJsonDocument doc(localisedMessages);
  AndroidVPNActivity::sendToService(
      ServiceAction::ACTION_SET_NOTIFICATION_FALLBACK, doc.toJson());
}