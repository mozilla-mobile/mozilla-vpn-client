#include "linuxcontroller.h"
#include "device.h"
#include "server.h"
#include "keys.h"

#include <QDebug>

void LinuxController::activate(const Server &server, const Device* device, const Keys* keys)
{
    Q_ASSERT(device);
    Q_ASSERT(keys);

    qDebug() << "LinuxController activated" << server.hostname();

    QString content;
    content.append("[Interface}\nPrivateKey = ");
    content.append(keys->privateKey());
    content.append("\nAddress = ");
    content.append(device->ipv4Address());
    content.append(", ");
    content.append(device->ipv6Address());
    content.append("\nDNS = ");
    content.append(server.ipv4Gateway());
    content.append("\n\n[Peer]\nPublicKey = ");
    content.append(server.publicKey());
    content.append("\nEndpoint = ");
    content.append(server.ipv4AddrIn());
    content.append(QString(":%1").arg(server.choosePort()));
    content.append("\nAllowedIPs = 0.0.0.0/0,::0/0\n");

    qDebug() << content;
}

void LinuxController::deactivate() {
    qDebug() << "LinuxController deactivated";
}
