    /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtrayhandler.h"
#include "logger.h"
#include "mozillavpn.h"
#include "statusicon.h"

#include <array>
#include <QIcon>
#include <QMenu>

namespace {
Logger logger(LOG_MAIN, "SystemTrayHandler");
}

SystemTrayHandler::SystemTrayHandler(QObject *parent)
    : QSystemTrayIcon(parent)
{
    //% "Show Mozilla VPN"
    m_menu.addAction(qtTrId("systray.show"), MozillaVPN::instance(), &MozillaVPN::showWindow);

    m_menu.addSeparator();

    //% "Quit"
    m_menu.addAction(qtTrId("systray.quit"), this, &SystemTrayHandler::quit);
    setContextMenu(&m_menu);

    iconChanged(MozillaVPN::instance()->statusIcon()->iconString());
}

void SystemTrayHandler::controllerStateChanged()
{
    logger.log() << "Show notification";

    if (!supportsMessages()) {
        return;
    }

    MozillaVPN *vpn = MozillaVPN::instance();

    // If we are in a non-main state.
    if (vpn->state() != MozillaVPN::StateMain) {
        return;
    }

    QString title;
    QString message;

    switch (vpn->controller()->state()) {
    case Controller::StateOn:
        //% "VPN Connected"
        title = qtTrId("vpn.systray.statusConnected.title");
        //% "Connected to %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is the city.
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
        break;

    case Controller::StateOff:
        //% "VPN Disconnected"
        title = qtTrId("vpn.systray.statusDisconnected.title");
        //% "Disconnected from to %1, %2"
        //: Shown as message body in a notification. %1 is the country, %2 is the city.
        message = qtTrId("vpn.systray.statusDisconnected.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
        break;

    case Controller::StateSwitching:
        //% "VPN Switched Servers"
        title = qtTrId("vpn.systray.statusSwitch.title");
        //% "Switched from %1, %2 to %3, %4"
        //: Shown as message body in a notification. %1 and %3 are countries, %2 and %4 are cities.
        message = qtTrId("vpn.systray.statusSwtich.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city())
                      .arg(vpn->serverCountryModel()->countryName(
                          vpn->controller()->switchingCountryCode()))
                      .arg(vpn->controller()->switchingCity());
        break;

    default:
        break;
    }

    Q_ASSERT(title.isEmpty() == message.isEmpty());
    if (!title.isEmpty()) {
        showMessage(title, message, NoIcon, 2000);
    }
}

void SystemTrayHandler::captivePortalNotificationRequested()
{
    logger.log() << "Capitve portal notification shown";
    //% "Captive portal detected"
    QString title = qtTrId("vpn.systray.captivePortalAlert.title");
    //% "VPN will automatically reconnect when ready"
    QString message = qtTrId("vpn.systray.captivePortalAlert.message");
    showMessage(title, message, NoIcon, 2000);
}

void SystemTrayHandler::iconChanged(const QString &icon)
{
    QIcon trayIconMask(icon);
    trayIconMask.setIsMask(true);
    setIcon(trayIconMask);
}
