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
    Q_DISABLE_COPY_MOVE(SystemTrayHandler)

public:
    static SystemTrayHandler *instance();

    explicit SystemTrayHandler(QObject *parent);
    ~SystemTrayHandler();

    void captivePortalNotificationRequested();

public slots:
    void updateIcon(const QString &icon);

    void updateContextMenu();

private:
    QMenu m_menu;

    QAction *m_statusLabel = nullptr;
    QAction *m_lastLocationLabel = nullptr;
    QAction *m_disconnectAction = nullptr;
    QAction *m_separator = nullptr;
    QAction *m_preferencesAction = nullptr;
};

#endif // SYSTEMTRAYHANDLER_H
