#include "dbus.h"
#include "polkithelper.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../wireguard-tools/contrib/embeddable-wg-library/wireguard.h"

#if defined(__cplusplus)
}
#endif

DBus::DBus(QObject *parent) : QObject(parent) {}

bool DBus::activate(const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    const QString &serverIpv6AddrIn,
                    int serverPort,
                    bool ipv6Enabled)
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
                      serverIpv6AddrIn,
                      serverPort,
                      ipv6Enabled);
}

bool DBus::deactivate(const QString &privateKey,
                      const QString &deviceIpv4Address,
                      const QString &deviceIpv6Address,
                      const QString &serverIpv4Gateway,
                      const QString &serverPublicKey,
                      const QString &serverIpv4AddrIn,
                      const QString &serverIpv6AddrIn,
                      int serverPort,
                      bool ipv6Enabled)
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
                      serverIpv6AddrIn,
                      serverPort,
                      ipv6Enabled);
}

QString DBus::status()
{
    qDebug() << "Status request";

    QJsonObject json;

    wg_device *device = nullptr;
    if (wg_get_device(&device, WG_INTERFACE) != 0) {
        qDebug() << "Unable to get device";
        json.insert("status", QJsonValue(false));
        return QJsonDocument(json).toJson();
    }

    uint32_t txBytes = 0;
    uint32_t rxBytes = 0;

    wg_peer *peer;
    wg_for_each_peer(device, peer)
    {
        txBytes += peer->tx_bytes;
        rxBytes += peer->rx_bytes;
    }

    wg_free_device(device);

    json.insert("status", QJsonValue(true));
    json.insert("txBytes", QJsonValue(qint64(txBytes)));
    json.insert("rxBytes", QJsonValue(qint64(rxBytes)));

    return QJsonDocument(json).toJson();
}

bool DBus::runWgQuick(WgQuickProcess::Op op,
                      const QString &privateKey,
                      const QString &deviceIpv4Address,
                      const QString &deviceIpv6Address,
                      const QString &serverIpv4Gateway,
                      const QString &serverPublicKey,
                      const QString &serverIpv4AddrIn,
                      const QString &serverIpv6AddrIn,
                      int serverPort,
                      bool ipv6Enabled)
{
    WgQuickProcess *wgQuick = new WgQuickProcess(op);

    wgQuick->run(privateKey,
                 deviceIpv4Address,
                 deviceIpv6Address,
                 serverIpv4Gateway,
                 serverPublicKey,
                 serverIpv4AddrIn,
                 serverIpv6AddrIn,
                 serverPort,
                 ipv6Enabled);

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
