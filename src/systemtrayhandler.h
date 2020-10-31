/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEMTRAYHANDLER_H
#define SYSTEMTRAYHANDLER_H

#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

class MozillaVPN;

class SystemTrayHandler final : public QSystemTrayIcon
{
    Q_OBJECT

public:
    SystemTrayHandler(const QIcon &icon, QObject *parent);

    void captivePortalNotificationRequested();

signals:
    void quit();

public slots:
    void controllerStateChanged();

private:
    QMenu m_menu;
};

#endif // SYSTEMTRAYHANDLER_H
