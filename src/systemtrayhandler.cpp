/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtrayhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "statusicon.h"

#include <array>
#include <QIcon>
#include <QMenu>

namespace {
Logger logger(LOG_MAIN, "SystemTrayHandler");

SystemTrayHandler* s_instance = nullptr;
}  // namespace

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

  MozillaVPN* vpn = MozillaVPN::instance();

  // Status label
  m_statusLabel = m_menu.addAction("");
  m_statusLabel->setEnabled(false);

  m_lastLocationLabel =
      m_menu.addAction("", vpn->controller(), &Controller::activate);
  m_lastLocationLabel->setEnabled(false);

  //% "Disconnect"
  m_disconnectAction = m_menu.addAction(
      qtTrId("systray.disconnect"), vpn->controller(), &Controller::deactivate);

  m_separator = m_menu.addSeparator();

  //% "Show Mozilla VPN"
  m_menu.addAction(qtTrId("systray.show"), QmlEngineHolder::instance(),
                   &QmlEngineHolder::showWindow);

  m_menu.addSeparator();

  //% "Help"
  QMenu* help = m_menu.addMenu(qtTrId("systray.help"));
  vpn->helpModel()->forEach([&](const QString& name, int id) {
    help->addAction(name, [help = vpn->helpModel(), id]() { help->open(id); });
  });

  //% "Preferences…"
  m_preferencesAction = m_menu.addAction(qtTrId("systray.preferences"), vpn,
                                         &MozillaVPN::requestSettings);

  m_menu.addSeparator();

  //% "Quit Mozilla VPN"
  m_menu.addAction(qtTrId("systray.quit"), vpn->controller(),
                   &Controller::quit);
  setContextMenu(&m_menu);

  updateIcon(MozillaVPN::instance()->statusIcon()->iconString());

  updateContextMenu();
}

SystemTrayHandler::~SystemTrayHandler() {
  MVPN_COUNT_DTOR(SystemTrayHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void SystemTrayHandler::updateContextMenu() {
  logger.log() << "Update context menu";

  MozillaVPN* vpn = MozillaVPN::instance();

  bool isStateMain = vpn->state() == MozillaVPN::StateMain;
  m_preferencesAction->setVisible(isStateMain);

  m_disconnectAction->setVisible(isStateMain && vpn->controller()->state() ==
                                                    Controller::StateOn);

  m_statusLabel->setVisible(isStateMain);
  m_lastLocationLabel->setVisible(isStateMain);

  m_separator->setVisible(isStateMain);

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
          .arg(vpn->currentServer()->country())
          .arg(vpn->currentServer()->city()));
  m_lastLocationLabel->setEnabled(vpn->controller()->state() ==
                                  Controller::StateOff);
}

void SystemTrayHandler::captivePortalNotificationRequested() {
  logger.log() << "Capitve portal notification shown";
  //% "Captive portal detected"
  QString title = qtTrId("vpn.systray.captivePortalAlert.title");
  //% "VPN will automatically reconnect when ready"
  QString message = qtTrId("vpn.systray.captivePortalAlert.message");
  showMessage(title, message, NoIcon, 2000);
}

void SystemTrayHandler::updateIcon(const QString& icon) {
  QIcon trayIconMask(icon);
  trayIconMask.setIsMask(true);
  setIcon(trayIconMask);
}
