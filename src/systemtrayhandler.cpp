/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtrayhandler.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QMenu>

SystemTrayHandler::SystemTrayHandler(const QIcon &icon, QObject *parent)
    : QSystemTrayIcon(icon, parent)
{
    m_menu.addAction(tr("quit"), this, &SystemTrayHandler::quit);
    setContextMenu(&m_menu);
}

void SystemTrayHandler::controllerStateChanged()
{
    qDebug() << "Show notification";

    if (!supportsMessages()) {
        return;
    }

    switch (MozillaVPN::instance()->controller()->state()) {
    case Controller::StateOn:
        showMessage(tr("Mozilla VPN connected"), tr("TODO"), NoIcon, 2000);
        break;

    case Controller::StateOff:
        showMessage(tr("Mozilla VPN disconnected"), tr("TODO"), NoIcon, 2000);
        break;

    case Controller::StateSwitching:
        showMessage(tr("Mozilla VPN switching"), tr("TODO"), NoIcon, 2000);
        break;

    default:
        // Nothing to do.
        break;
    }
}
