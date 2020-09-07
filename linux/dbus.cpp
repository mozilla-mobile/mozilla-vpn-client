#include "dbus.h"
#include "polkithelper.h"

#include <QCoreApplication>

DBus::DBus(QObject *parent) : QObject(parent) {}

bool DBus::activate(const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    int serverPort)
{
    if (!PolkitHelper::instance()->checkAuthorization("org.mozilla.vpn.activate")) {
        return false;
    }

    return runWgQuick(WgQuickProcess::Up,
                      privateKey,
                      deviceIpv4Address,
                      deviceIpv6Address,
                      serverIpv4Gateway,
                      serverPublicKey,
                      serverIpv4AddrIn,
                      serverPort);
}

bool DBus::deactivate(const QString &privateKey,
                      const QString &deviceIpv4Address,
                      const QString &deviceIpv6Address,
                      const QString &serverIpv4Gateway,
                      const QString &serverPublicKey,
                      const QString &serverIpv4AddrIn,
                      int serverPort)
{
    if (!PolkitHelper::instance()->checkAuthorization("org.mozilla.vpn.deactivate")) {
        return false;
    }

    return runWgQuick(WgQuickProcess::Down,
                      privateKey,
                      deviceIpv4Address,
                      deviceIpv6Address,
                      serverIpv4Gateway,
                      serverPublicKey,
                      serverIpv4AddrIn,
                      serverPort);
}

bool DBus::runWgQuick(WgQuickProcess::Op op,
                      const QString &privateKey,
                      const QString &deviceIpv4Address,
                      const QString &deviceIpv6Address,
                      const QString &serverIpv4Gateway,
                      const QString &serverPublicKey,
                      const QString &serverIpv4AddrIn,
                      int serverPort)
{
    WgQuickProcess *wgQuick = new WgQuickProcess(op);

    wgQuick->run(privateKey,
                 deviceIpv4Address,
                 deviceIpv6Address,
                 serverIpv4Gateway,
                 serverPublicKey,
                 serverIpv4AddrIn,
                 serverPort);

    enum Result {
        Pending,
        Success,
        Fail,
    };

    Result result = Pending;
    connect(wgQuick, &WgQuickProcess::failed, [&] { result = Fail; });
    connect(wgQuick, &WgQuickProcess::succeeded, [&] { result = Success; });

    while (result == Pending) {
        QCoreApplication::processEvents();
    }

    return result == Success;
}
