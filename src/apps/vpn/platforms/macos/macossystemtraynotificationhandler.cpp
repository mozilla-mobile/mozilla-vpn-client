/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macossystemtraynotificationhandler.h"

#include <QIcon>
#include <QMenu>
#include <QWindow>

#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "macosutils.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "statusicon.h"

namespace {
Logger logger("MacosSystemTrayNotificationHandler");
}

MacosSystemTrayNotificationHandler::MacosSystemTrayNotificationHandler(
    QObject* parent)
    : SystemTrayNotificationHandler(parent) {
  MZ_COUNT_CTOR(MacosSystemTrayNotificationHandler);
}

MacosSystemTrayNotificationHandler::~MacosSystemTrayNotificationHandler() {
  MZ_COUNT_DTOR(MacosSystemTrayNotificationHandler);
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
  m_macOSStatusIcon->setToolTip(I18nStrings::instance()->t(I18nStrings::ProductName));
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
  m_macOSStatusIcon->showMessage(title, message);
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
