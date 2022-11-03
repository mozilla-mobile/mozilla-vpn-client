/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macossystemtraynotificationhandler.h"
#include "macosutils.h"

#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QIcon>
#include <QMenu>
#include <QWindow>

namespace {
Logger logger(LOG_MAIN, "MacosSystemTrayNotificationHandler");
}

MacosSystemTrayNotificationHandler::MacosSystemTrayNotificationHandler(
    QObject* parent)
    : SystemTrayNotificationHandler(parent) {
  MVPN_COUNT_CTOR(MacosSystemTrayNotificationHandler);
}

MacosSystemTrayNotificationHandler::~MacosSystemTrayNotificationHandler() {
  MVPN_COUNT_DTOR(MacosSystemTrayNotificationHandler);
}

void MacosSystemTrayNotificationHandler::initialize() {
  SystemTrayNotificationHandler::initialize();
}

void MacosSystemTrayNotificationHandler::setStatusMenu() {
  logger.debug() << "Set status menu";

  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn->statusIcon(), &StatusIcon::iconUpdateNeeded, this,
          &MacosSystemTrayNotificationHandler::updateIconIndicator);

  m_macOSStatusIcon = new MacOSStatusIcon(this);
  m_macOSStatusIcon->setToolTip(qtTrId("vpn.main.productName"));
  m_macOSStatusIcon->setMenu(m_menu->toNSMenu());
}

void MacosSystemTrayNotificationHandler::showHideWindow() {
  logger.debug() << "Show/hide window";

  QmlEngineHolder* engine = QmlEngineHolder::instance();
  if (engine->window()->isVisible()) {
    engine->hideWindow();
    MacOSUtils::hideDockIcon();
  } else {
    engine->showWindow();
    MacOSUtils::showDockIcon();
  }
}

void MacosSystemTrayNotificationHandler::notify(Message type,
                                                const QString& title,
                                                const QString& message,
                                                int timerMsec) {
  // This is very hacky, but necessary to circumvent a Qt bug where
  // notifications are not shown when the icon is not visible.
  // See: https://bugreports.qt.io/browse/QTBUG-108134
  m_systemTrayIcon->show();
  SystemTrayNotificationHandler::notify(type, title, message, timerMsec);
  m_systemTrayIcon->hide();
}

void MacosSystemTrayNotificationHandler::updateIcon() {
  logger.debug() << "Update icon";

  MozillaVPN* vpn = MozillaVPN::instance();
  m_macOSStatusIcon->setIcon(vpn->statusIcon()->iconString());
}

void MacosSystemTrayNotificationHandler::updateIconIndicator() {
  logger.debug() << "Update icon indicator";

  MozillaVPN* vpn = MozillaVPN::instance();
  m_macOSStatusIcon->setIndicatorColor(vpn->statusIcon()->indicatorColor());
}
