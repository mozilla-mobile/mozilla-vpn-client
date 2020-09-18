#ifndef SYSTEMTRAYHANDLER_H
#define SYSTEMTRAYHANDLER_H

#include <QMenu>
#include <QSystemTrayIcon>

class MozillaVPN;

class SystemTrayHandler : public QSystemTrayIcon
{
    Q_OBJECT

public:
    SystemTrayHandler(const QIcon &icon, QObject *parent);

    void notificationRequired(MozillaVPN *vpn);

signals:
    void quit();

private:
    QMenu m_menu;
};

#endif // SYSTEMTRAYHANDLER_H
