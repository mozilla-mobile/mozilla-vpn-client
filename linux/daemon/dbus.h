#ifndef DBUS_H
#define DBUS_H

#include "wgquickprocess.h"

#include <QObject>

class DBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mozilla.vpn.dbus")

public:
    DBus(QObject *parent);

public Q_SLOTS:
    bool activate(const QString &privateKey,
                  const QString &deviceIpv4Address,
                  const QString &deviceIpv6Address,
                  const QString &serverIpv4Gateway,
                  const QString &serverPublicKey,
                  const QString &serverIpv4AddrIn,
                  const QString &serverIpv6AddrIn,
                  int serverPort,
                  bool ipv6Enabled);

    bool deactivate(const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    const QString &serverIpv6AddrIn,
                    int serverPort,
                    bool ipv6Enabled);

    QString status();

private:
    bool runWgQuick(WgQuickProcess::Op op,
                    const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    const QString &serverIpv6AddrIn,
                    int serverPort,
                    bool ipv6Enabled);
};

#endif // DBUS_H
