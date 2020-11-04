/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEMTRAYHANDLER_H
#define SYSTEMTRAYHANDLER_H

#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

class MozillaVPN;
class QAction;

class SystemTrayHandler final : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTrayHandler(QObject *parent);

    void captivePortalNotificationRequested();

signals:
    void quit();

public slots:
    void iconChanged(const QString &icon);
    void controllerStateChanged();

private:
    QMenu m_menu;
    QAction *m_preferencesAction = nullptr;
};

#endif // SYSTEMTRAYHANDLER_H
