/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

#include "helper.h"
#include "systemtraynotificationhandler.h"

SystemTrayNotificationHandler::SystemTrayNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  // We must use these private fields somehow to avoid Wunused-private-field
  // warnings/errors.
  m_statusLabel = nullptr;
  m_lastLocationLabel = nullptr;
  m_disconnectAction = nullptr;
  m_separator = nullptr;
  m_showHideLabel = nullptr;
  m_quitAction = nullptr;
}

SystemTrayNotificationHandler::~SystemTrayNotificationHandler() {}

void SystemTrayNotificationHandler::initialize() {}

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
