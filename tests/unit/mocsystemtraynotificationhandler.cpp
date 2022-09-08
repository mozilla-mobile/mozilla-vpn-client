/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtraynotificationhandler.h"
#include "helper.h"

#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>

SystemTrayNotificationHandler::SystemTrayNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {}

SystemTrayNotificationHandler::~SystemTrayNotificationHandler() {}

void SystemTrayNotificationHandler::createStatusMenu() {}

void SystemTrayNotificationHandler::setStatusMenu() {}

void SystemTrayNotificationHandler::notify(NotificationHandler::Message type,
                                           const QString& title,
                                           const QString& message,
                                           int timerMsec) {
  TestHelper::SystemNotification notification;
  notification.type = type;
  notification.title = title;
  notification.message = message;
  notification.timer = timerMsec;

  TestHelper::lastSystemNotification = notification;
}

void SystemTrayNotificationHandler::retranslate() {}

void SystemTrayNotificationHandler::updateContextMenu() {}

void SystemTrayNotificationHandler::updateIcon() {}

void SystemTrayNotificationHandler::showHideWindow() {}

void SystemTrayNotificationHandler::maybeActivated(
    QSystemTrayIcon::ActivationReason reason) {}
