#ifndef DBUS_H
#define DBUS_H

#include "../../src/platforms/linux/wgquickprocess.h"

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
                  int serverPort);
    bool deactivate(const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    int serverPort);

private:
    bool runWgQuick(WgQuickProcess::Op op,
                    const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    int serverPort);
};

#endif // DBUS_H
