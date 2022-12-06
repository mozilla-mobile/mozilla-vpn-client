/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtraynotificationhandler.h"

#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>

SystemTrayNotificationHandler::SystemTrayNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {}

SystemTrayNotificationHandler::~SystemTrayNotificationHandler() {}

void SystemTrayNotificationHandler::initialize() {}

void SystemTrayNotificationHandler::createStatusMenu() {}

void SystemTrayNotificationHandler::setStatusMenu() {}

void SystemTrayNotificationHandler::notifyInternal(
    NotificationHandler::Message type, const QString& title,
    const QString& message, int timerMsec) {
  Q_UNUSED(type);
  Q_UNUSED(title);
  Q_UNUSED(message);
  Q_UNUSED(timerMsec);
}

void SystemTrayNotificationHandler::retranslate() {}

void SystemTrayNotificationHandler::updateContextMenu() {}

void SystemTrayNotificationHandler::updateIcon() {}

void SystemTrayNotificationHandler::showHideWindow() {}

void SystemTrayNotificationHandler::maybeActivated(
    QSystemTrayIcon::ActivationReason reason) {}
