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
    = {"://ui/resources/logo-connecting1.svg",
       "://ui/resources/logo-connecting2.svg",
       "://ui/resources/logo-connecting3.svg",
       "://ui/resources/logo-connecting4.svg"};

namespace {
Logger logger(LOG_MAIN, "SystemTrayHandler");
}

SystemTrayHandler::SystemTrayHandler(const QIcon &icon, QObject *parent)
    : QSystemTrayIcon(icon, parent)
{   //% "Quit"
    m_menu.addAction(qtTrId("systray.quit"), this, &SystemTrayHandler::quit);
    setContextMenu(&m_menu);

    connect(&m_animatedIconTimer, &QTimer::timeout, [this]() {
        Q_ASSERT(m_animatedIconIndex < ANIMATED_ICON_STEPS.size());
        setIcon(QIcon(ANIMATED_ICON_STEPS[m_animatedIconIndex++]));
        if (m_animatedIconIndex == ANIMATED_ICON_STEPS.size()) {
            m_animatedIconIndex = 0;
        }
    });
}

void SystemTrayHandler::controllerStateChanged()
{
    logger.log() << "Show notification";

    if (!supportsMessages()) {
        return;
    }

    switch (MozillaVPN::instance()->controller()->state()) {
    case Controller::StateOn:

        showIcon("://ui/resources/logo-on.svg");
        //% "Mozilla VPN connected"
        showMessage(qtTrId("vpn.systray.statusConnected"), qtTrId("TODO"), NoIcon, 2000);
        break;

    case Controller::StateOff:
        showIcon("://ui/resources/logo-tray.svg");
        //% "Mozilla VPN disconnected"
        showMessage(qtTrId("vpn.systray.statusDisconnected"), qtTrId("TODO"), NoIcon, 2000);
        break;

    case Controller::StateSwitching:
        showIcon("://ui/resources/logo-tray.svg");
        //% "Mozilla VPN switching"
        //: This message is shown when the VPN is switching to a different server in a different location.
        showMessage(qtTrId("vpn.systray.statusSwitch"), qtTrId("TODO"), NoIcon, 2000);
        break;

    case Controller::StateConnecting:
        animateConnectingIcon();
        break;

    default:
        showIcon("://ui/resources/logo-tray.svg");
        break;
    }
}

void SystemTrayHandler::captivePortalNotificationRequested()
{
    logger.log() << "Capitve portal notification shown";
    //% "Captive portal detected"
    showMessage(qtTrId("vpn.systray.captivePortalAlert"), tr("TODO"), NoIcon, 2000);
}

void SystemTrayHandler::animateConnectingIcon()
{
    m_animatedIconTimer.start(200);
}

void SystemTrayHandler::showIcon(const QString &icon)
{
    m_animatedIconTimer.stop();
    setIcon(QIcon(icon));
}
