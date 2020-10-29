/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtrayhandler.h"
#include "logger.h"
#include "mozillavpn.h"

#include <array>
#include <QIcon>
#include <QMenu>

constexpr const std::array<const char *, 4> ANIMATED_ICON_STEPS
    = {"://ui/resources/logo-animated1.svg",
       "://ui/resources/logo-animated2.svg",
       "://ui/resources/logo-animated3.svg",
       "://ui/resources/logo-animated4.svg"};

namespace {
Logger logger(LOG_MAIN, "SystemTrayHandler");
}

SystemTrayHandler::SystemTrayHandler(const QIcon &icon, QObject *parent)
    : QSystemTrayIcon(icon, parent)
{   //% "Quit"
    m_menu.addAction(qtTrId("systray.quit"), this, &SystemTrayHandler::quit);
    setContextMenu(&m_menu);

    connect(&m_animatedIconTimer, &QTimer::timeout, this, &SystemTrayHandler::animateIcon);
}

void SystemTrayHandler::controllerStateChanged()
{
    logger.log() << "Show notification";

    if (!supportsMessages()) {
        return;
    }

    MozillaVPN *vpn = MozillaVPN::instance();

    // If we are in a non-main state, we don't need to show special icons.
    if (vpn->state() != MozillaVPN::StateMain) {
        showIcon("://ui/resources/logo-tray.svg");
        return;
    }

    QString title;
    QString message;

    switch (vpn->controller()->state()) {
    case Controller::StateOn:
        showIcon("://ui/resources/logo-on.svg");
        //% "VPN connected"
        title = qtTrId("vpn.systray.statusConnected.title");
        //% "Connected to %1 - %2";
        //: Shown as message body in a notification. %1 is the country, %2 is the city.
        message = qtTrId("vpn.systray.statusConnected.message")
                      .arg(vpn->currentServer()->country())
                      .arg(vpn->currentServer()->city());
        break;

    case Controller::StateOff:
        showIcon("://ui/resources/logo-off.svg");
        //% "VPN disconnected"
        title = qtTrId("vpn.systray.statusDisconnected");
        message = qtTrId("TODO");
        break;

    case Controller::StateSwitching:
        showAnimatedIcon();
        //% "VPN switching"
        //: This message is shown when the VPN is switching to a different server in a different location.
        title = qtTrId("vpn.systray.statusSwitch");
        message = qtTrId("TODO");
        break;

    case Controller::StateConnecting:
        [[fallthrough]];
    case Controller::StateDisconnecting:
        showAnimatedIcon();
        break;

    default:
        showIcon("://ui/resources/logo-tray.svg");
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
    showMessage(qtTrId("vpn.systray.captivePortalAlert"), qtTrId("TODO"), NoIcon, 2000);
}

void SystemTrayHandler::showAnimatedIcon()
{
    m_animatedIconIndex = 0;
    m_animatedIconTimer.start(200);
    animateIcon();
}

void SystemTrayHandler::animateIcon()
{
    Q_ASSERT(m_animatedIconIndex < ANIMATED_ICON_STEPS.size());
    setIcon(QIcon(ANIMATED_ICON_STEPS[m_animatedIconIndex++]));
    if (m_animatedIconIndex == ANIMATED_ICON_STEPS.size()) {
        m_animatedIconIndex = 0;
    }
}

void SystemTrayHandler::showIcon(const QString &icon)
{
    m_animatedIconTimer.stop();
    setIcon(QIcon(icon));
}
