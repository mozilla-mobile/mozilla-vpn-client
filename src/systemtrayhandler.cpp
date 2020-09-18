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

void SystemTrayHandler::notificationRequired(MozillaVPN *vpn)
{
    qDebug() << "Show notification";

    if (!supportsMessages()) {
        return;
    }

    switch (vpn->controller()->state()) {
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
