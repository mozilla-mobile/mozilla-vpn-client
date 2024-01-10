/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidnotificationhandler.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "androidvpnactivity.h"
#include "i18nstrings.h"
#include "logging/logger.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("AndroidNotificationHandler");
}

AndroidNotificationHandler::AndroidNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MZ_COUNT_CTOR(AndroidNotificationHandler);
}

AndroidNotificationHandler::~AndroidNotificationHandler() {
  MZ_COUNT_DTOR(AndroidNotificationHandler);
}

void AndroidNotificationHandler::initialize() {}

void AndroidNotificationHandler::notify(NotificationHandler::Message type,
                                        const QString& title,
                                        const QString& message, int timerMsec) {
  Q_UNUSED(type)
  Q_UNUSED(timerMsec)
  logger.debug() << "Send notification - " << message;
  QJsonObject args;
  args["header"] = title;
  args["body"] = message;
  QJsonDocument doc(args);
  AndroidVPNActivity::sendToService(ServiceAction::ACTION_SET_NOTIFICATION_TEXT,
                                    doc.toJson(QJsonDocument::Compact));
}
