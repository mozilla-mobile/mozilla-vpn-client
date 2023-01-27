/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtraynotificationhandler.h"

#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWindow>

#include "appconstants.h"
#include "externalophandler.h"
#include "frontend/navigator.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

namespace {
Logger logger("SystemTrayNotificationHandler");
}

SystemTrayNotificationHandler::SystemTrayNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MZ_COUNT_CTOR(SystemTrayNotificationHandler);
}

SystemTrayNotificationHandler::~SystemTrayNotificationHandler() {
  MZ_COUNT_DTOR(SystemTrayNotificationHandler);
}

void SystemTrayNotificationHandler::initialize() {
  m_menu.reset(new QMenu());
  m_systemTrayIcon = new QSystemTrayIcon(parent());

  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn, &MozillaVPN::stateChanged, this,
          &SystemTrayNotificationHandler::updateContextMenu);

  connect(vpn->serverData(), &ServerData::changed, this,
          &SystemTrayNotificationHandler::updateContextMenu);

  connect(vpn->controller(), &Controller::stateChanged, this,
          &SystemTrayNotificationHandler::updateContextMenu);

  connect(vpn->statusIcon(), &StatusIcon::iconUpdateNeeded, this,
          &SystemTrayNotificationHandler::updateIcon);

  connect(QmlEngineHolder::instance()->window(), &QWindow::visibleChanged, this,
          &SystemTrayNotificationHandler::updateContextMenu);

  connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this,
          &SystemTrayNotificationHandler::maybeActivated);

  connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked, this,
          &SystemTrayNotificationHandler::messageClickHandle);

  // Initial setup for the systray
  createStatusMenu();
  setStatusMenu();
  retranslate();
  updateIcon();
}

#ifdef MZ_WASM
QMenu* SystemTrayNotificationHandler::contextMenu() {
  return m_systemTrayIcon->contextMenu();
}
#endif

void SystemTrayNotificationHandler::createStatusMenu() {
  logger.debug() << "Create status menu";

  m_statusLabel = m_menu->addAction("");
  m_statusLabel->setEnabled(false);

  m_lastLocationLabel = m_menu->addAction("", []() {
    (void)ExternalOpHandler::instance()->request(ExternalOpHandler::OpActivate);
  });
  m_lastLocationLabel->setEnabled(false);

  m_disconnectAction = m_menu->addAction("", []() {
    (void)ExternalOpHandler::instance()->request(
        ExternalOpHandler::OpDeactivate);
  });

  m_separator = m_menu->addSeparator();

  m_showHideLabel = m_menu->addAction(
      "", this, &SystemTrayNotificationHandler::showHideWindow);

  m_menu->addSeparator();

  m_quitAction = m_menu->addAction("", []() {
    (void)ExternalOpHandler::instance()->request(ExternalOpHandler::OpQuit);
  });
}

void SystemTrayNotificationHandler::setStatusMenu() {
  logger.debug() << "Set status menu";

  // TODO: Check if method is called on these devices.
#if defined(MZ_LINUX) || defined(MZ_WINDOWS)
  m_systemTrayIcon->setToolTip(qtTrId("vpn.main.productName"));
  m_systemTrayIcon->setContextMenu(m_menu.get());
  m_systemTrayIcon->show();
#endif
}

void SystemTrayNotificationHandler::notify(NotificationHandler::Message type,
                                           const QString& title,
                                           const QString& message,
                                           int timerMsec) {
  Q_UNUSED(type);

  QIcon icon(AppConstants::LOGO_URL);
  m_systemTrayIcon->showMessage(title, message, icon, timerMsec);
}

void SystemTrayNotificationHandler::retranslate() {
  logger.debug() << "Retranslate";

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  m_disconnectAction->setText(l18nStrings->t(L18nStrings::SystrayDisconnect));
  m_quitAction->setText(l18nStrings->t(L18nStrings::SystrayQuit));

  updateContextMenu();
}

void SystemTrayNotificationHandler::updateContextMenu() {
  logger.debug() << "Update context menu";

#if defined(MZ_IOS) || defined(MZ_ANDROID)
  return;
#endif

  // If the QML Engine Holder has been released, we are shutting down.
  if (!QmlEngineHolder::exists()) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();

  bool isStateMain = vpn->state() == MozillaVPN::StateMain;

  m_disconnectAction->setVisible(isStateMain && vpn->controller()->state() ==
                                                    Controller::StateOn);

  m_statusLabel->setVisible(isStateMain);
  m_lastLocationLabel->setVisible(isStateMain);

  m_separator->setVisible(isStateMain);

  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  if (QmlEngineHolder::instance()->window()->isVisible()) {
    m_showHideLabel->setText(l18nStrings->t(L18nStrings::SystrayHide));
  } else {
    m_showHideLabel->setText(l18nStrings->t(L18nStrings::SystrayShow));
  }

  // If we are in a non-main state, we don't need to show notifications.
  if (!isStateMain) {
    return;
  }

  QString statusLabel;

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      [[fallthrough]];
    case Controller::StateSilentSwitching:
      statusLabel = l18nStrings->t(L18nStrings::SystrayStatusConnectedTo);
      break;

    case Controller::StateOff:
      statusLabel = l18nStrings->t(L18nStrings::SystrayStatusConnectTo);
      break;

    case Controller::StateSwitching:
      [[fallthrough]];
    case Controller::StateConnecting:
      [[fallthrough]];
    case Controller::StateConfirming:
      statusLabel = l18nStrings->t(L18nStrings::SystrayStatusConnectingTo);
      break;

    case Controller::StateDisconnecting:
      statusLabel = l18nStrings->t(L18nStrings::SystrayStatusDisconnectingFrom);
      break;

    default:
      m_statusLabel->setVisible(false);
      m_lastLocationLabel->setVisible(false);
      m_separator->setVisible(false);
      return;
  }

  Q_ASSERT(!statusLabel.isEmpty());
  m_statusLabel->setVisible(true);
  m_statusLabel->setText(statusLabel);

  m_lastLocationLabel->setVisible(true);

  QIcon flagIcon(QString(":/nebula/resources/flags/%1.png")
                     .arg(vpn->serverData()->exitCountryCode().toUpper()));

  QString localizedCityName = vpn->serverData()->localizedExitCityName();
  QString localizedCountryName = vpn->serverData()->localizedExitCountryName();

  m_lastLocationLabel->setIcon(flagIcon);
  m_lastLocationLabel->setText(
      l18nStrings->t(L18nStrings::SystrayLocation2)
          .arg(localizedCountryName, localizedCityName));
  m_lastLocationLabel->setEnabled(vpn->controller()->state() ==
                                  Controller::StateOff);
}

void SystemTrayNotificationHandler::updateIcon() {
  logger.debug() << "Update icon";

#if defined(MZ_LINUX) || defined(MZ_WINDOWS)
  MozillaVPN* vpn = MozillaVPN::instance();
  m_systemTrayIcon->setIcon(vpn->statusIcon()->icon());
#endif
}

void SystemTrayNotificationHandler::showHideWindow() {
  QmlEngineHolder* engine = QmlEngineHolder::instance();
  if (engine->window()->isVisible()) {
    engine->hideWindow();
  } else {
    engine->showWindow();
  }
}

void SystemTrayNotificationHandler::maybeActivated(
    QSystemTrayIcon::ActivationReason reason) {
  logger.debug() << "Activated";

#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
  if (reason == QSystemTrayIcon::DoubleClick ||
      reason == QSystemTrayIcon::Trigger) {
    QmlEngineHolder* engine = QmlEngineHolder::instance();
    engine->showWindow();
  }
#else
  Q_UNUSED(reason);
#endif
}
