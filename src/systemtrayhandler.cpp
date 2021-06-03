/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtrayhandler.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "statusicon.h"

#ifdef MVPN_LINUX
#  include "platforms/linux/linuxsystemtrayhandler.h"
#endif

#ifdef MVPN_MACOS
#  include "platforms/macos/macosutils.h"
#endif

#include <array>
#include <QIcon>
#include <QMenu>
#include <QWindow>

namespace {
Logger logger(LOG_MAIN, "SystemTrayHandler");

SystemTrayHandler* s_instance = nullptr;
}  // namespace

// static
SystemTrayHandler* SystemTrayHandler::create(QObject* parent) {
#if defined(MVPN_LINUX)
  if (LinuxSystemTrayHandler::requiredCustomImpl()) {
    return new LinuxSystemTrayHandler(parent);
  }
#endif

  return new SystemTrayHandler(parent);
}

// static
SystemTrayHandler* SystemTrayHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

SystemTrayHandler::SystemTrayHandler(QObject* parent)
    : QSystemTrayIcon(parent) {
  MVPN_COUNT_CTOR(SystemTrayHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;

#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
  return;
#endif

  MozillaVPN* vpn = MozillaVPN::instance();

  setToolTip(qtTrId("vpn.main.productName"));

  // Status label
  m_statusLabel = m_menu.addAction("");
  m_statusLabel->setEnabled(false);

  m_lastLocationLabel =
      m_menu.addAction("", vpn->controller(), &Controller::activate);
  m_lastLocationLabel->setEnabled(false);

  m_disconnectAction =
      m_menu.addAction("", vpn->controller(), &Controller::deactivate);

  m_separator = m_menu.addSeparator();

  m_showHideLabel =
      m_menu.addAction("", this, &SystemTrayHandler::showHideWindow);

  m_menu.addSeparator();

  m_helpMenu = m_menu.addMenu("");

  m_preferencesAction = m_menu.addAction("", vpn, &MozillaVPN::requestSettings);

  m_menu.addSeparator();

  m_quitAction = m_menu.addAction("", vpn->controller(), &Controller::quit);
  setContextMenu(&m_menu);

  updateIcon(MozillaVPN::instance()->statusIcon()->iconString());

  connect(QmlEngineHolder::instance()->window(), &QWindow::visibleChanged, this,
          &SystemTrayHandler::updateContextMenu);

  connect(this, &QSystemTrayIcon::activated, this,
          &SystemTrayHandler::maybeActivated);

  connect(this, &QSystemTrayIcon::messageClicked, this,
          &SystemTrayHandler::messageClickHandle);

  retranslate();
}

SystemTrayHandler::~SystemTrayHandler() {
  MVPN_COUNT_DTOR(SystemTrayHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void SystemTrayHandler::updateContextMenu() {
  logger.log() << "Update context menu";

#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
  return;
#endif

  // If the QML Engine Holder has been released, we are shutting down.
  if (!QmlEngineHolder::exists()) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();

  bool isStateMain = vpn->state() == MozillaVPN::StateMain;
  m_preferencesAction->setVisible(isStateMain);

  m_disconnectAction->setVisible(isStateMain && vpn->controller()->state() ==
                                                    Controller::StateOn);

  m_statusLabel->setVisible(isStateMain);
  m_lastLocationLabel->setVisible(isStateMain);

  m_separator->setVisible(isStateMain);

  if (QmlEngineHolder::instance()->window()->isVisible()) {
    //% "Hide Mozilla VPN"
    m_showHideLabel->setText(qtTrId("systray.hide"));
  } else {
    //% "Show Mozilla VPN"
    m_showHideLabel->setText(qtTrId("systray.show"));
  }

  // If we are in a non-main state, we don't need to show notifications.
  if (!isStateMain) {
    return;
  }

  QString statusLabel;

  switch (vpn->controller()->state()) {
    case Controller::StateOn:
      //% "Connected to:"
      statusLabel = qtTrId("vpn.systray.status.connectedTo");
      break;

    case Controller::StateOff:
      //% "Connect to the last location:"
      statusLabel = qtTrId("vpn.systray.status.connectTo");
      break;

    case Controller::StateSwitching:
      [[fallthrough]];
    case Controller::StateConnecting:
      [[fallthrough]];
    case Controller::StateConfirming:
      //% "Connecting to:"
      statusLabel = qtTrId("vpn.systray.status.connectingTo");
      break;

    case Controller::StateDisconnecting:
      //% "Disconnecting from:"
      statusLabel = qtTrId("vpn.systray.status.disconnectingFrom");
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

  QIcon flagIcon(QString(":/ui/resources/flags/%1.png")
                     .arg(vpn->currentServer()->countryCode().toUpper()));

  m_lastLocationLabel->setIcon(flagIcon);
  m_lastLocationLabel->setText(
      //% "%1, %2"
      //: Location in the systray. %1 is the country, %2 is the city.
      qtTrId("vpn.systray.location")
          .arg(vpn->currentServer()->countryName())
          .arg(vpn->currentServer()->cityName()));
  m_lastLocationLabel->setEnabled(vpn->controller()->state() ==
                                  Controller::StateOff);
}

void SystemTrayHandler::unsecuredNetworkNotification(
    const QString& networkName) {
  logger.log() << "Unsecured network notification shown";

  //% "Unsecured Wi-Fi network detected"
  QString title = qtTrId("vpn.systray.unsecuredNetwork.title");

  //% "%1 is not secure. Click here to turn on VPN and secure your device."
  //: %1 is the Wi-Fi network name
  QString message =
      qtTrId("vpn.systray.unsecuredNetwork2.message").arg(networkName);

  showNotificationInternal(UnsecuredNetwork, title, message,
                           Constants::UNSECURED_NETWORK_ALERT_MSEC);
}

void SystemTrayHandler::captivePortalBlockNotificationRequired() {
  logger.log() << "Captive portal block notification shown";

  //% "Guest Wi-Fi portal blocked"
  QString title = qtTrId("vpn.systray.captivePortalBlock.title");

  //% "The guest Wi-Fi network you’re connected to requires action. Click here"
  //% " to turn off VPN to see the portal."
  QString message = qtTrId("vpn.systray.captivePortalBlock2.message");

  showNotificationInternal(CaptivePortalBlock, title, message,
                           Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void SystemTrayHandler::captivePortalUnblockNotificationRequired() {
  logger.log() << "Captive portal unblock notification shown";

  //% "Guest Wi-Fi portal detected"
  QString title = qtTrId("vpn.systray.captivePortalUnblock.title");

  //% "The guest Wi-Fi network you’re connected to may not be secure. Click"
  //% " here to turn on VPN to secure your device."
  QString message = qtTrId("vpn.systray.captivePortalUnblock2.message");

  showNotificationInternal(CaptivePortalUnblock, title, message,
                           Constants::CAPTIVE_PORTAL_ALERT_MSEC);
}

void SystemTrayHandler::updateIcon(const QString& icon) {
  QIcon trayIconMask(icon);
  trayIconMask.setIsMask(true);
  setIcon(trayIconMask);
}

void SystemTrayHandler::showHideWindow() {
  QmlEngineHolder* engine = QmlEngineHolder::instance();
  if (engine->window()->isVisible()) {
    engine->hideWindow();
#ifdef MVPN_MACOS
    MacOSUtils::hideDockIcon();
#endif
  } else {
    engine->showWindow();
#ifdef MVPN_MACOS
    MacOSUtils::showDockIcon();
#endif
  }
}

void SystemTrayHandler::retranslate() {
  logger.log() << "Retranslate";

#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
  return;
#endif

  //% "Disconnect"
  m_disconnectAction->setText(qtTrId("systray.disconnect"));

  //% "Help"
  m_helpMenu->setTitle(qtTrId("systray.help"));
  for (QAction* action : m_helpMenu->actions()) {
    m_helpMenu->removeAction(action);
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  vpn->helpModel()->forEach([&](const char* nameId, int id) {
    m_helpMenu->addAction(qtTrId(nameId),
                          [help = vpn->helpModel(), id]() { help->open(id); });
  });

  //% "Preferences…"
  m_preferencesAction->setText(qtTrId("systray.preferences"));

  //% "Quit Mozilla VPN"
  m_quitAction->setText(qtTrId("systray.quit"));

  updateContextMenu();
}

void SystemTrayHandler::maybeActivated(
    QSystemTrayIcon::ActivationReason reason) {
  logger.log() << "Activated";

#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX)
  if (reason == QSystemTrayIcon::DoubleClick ||
      reason == QSystemTrayIcon::Trigger) {
    QmlEngineHolder* engine = QmlEngineHolder::instance();
    engine->showWindow();
  }
#else
  Q_UNUSED(reason);
#endif
}

void SystemTrayHandler::messageClickHandle() {
  logger.log() << "Message clicked";

  if (m_lastMessage == None) {
    logger.log() << "Random message clicked received";
    return;
  }

  emit notificationClicked(m_lastMessage);
  m_lastMessage = None;
}

void SystemTrayHandler::showNotification(const QString& title,
                                         const QString& message,
                                         int timerMsec) {
  showNotificationInternal(None, title, message, timerMsec);
}

void SystemTrayHandler::showNotificationInternal(Message type,
                                                 const QString& title,
                                                 const QString& message,
                                                 int timerMsec) {
  m_lastMessage = type;

  emit notificationShown(title, message);

  QIcon icon(Constants::LOGO_URL);
  showMessage(title, message, icon, timerMsec);
}
