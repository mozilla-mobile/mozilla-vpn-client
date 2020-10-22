/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

// static
Server Server::fromJson(QJsonObject &obj)
{
    QJsonValue hostname = obj.take("hostname");
    Q_ASSERT(hostname.isString());

    QJsonValue ipv4AddrIn = obj.take("ipv4_addr_in");
    Q_ASSERT(ipv4AddrIn.isString());

    QJsonValue ipv4Gateway = obj.take("ipv4_gateway");
    Q_ASSERT(ipv4Gateway.isString());

    QJsonValue ipv6AddrIn = obj.take("ipv6_addr_in");
    // If this object comes from the IOS migration, the ipv6_addr_in is missing.

    QJsonValue ipv6Gateway = obj.take("ipv6_gateway");
    Q_ASSERT(ipv6Gateway.isString());

    QJsonValue publicKey = obj.take("public_key");
    Q_ASSERT(publicKey.isString());

    QJsonValue weight = obj.take("weight");
    Q_ASSERT(weight.isDouble());

    Server s(hostname.toString(),
             ipv4AddrIn.toString(),
             ipv4Gateway.toString(),
             ipv6AddrIn.toString(),
             ipv6Gateway.toString(),
             publicKey.toString(),
             weight.toInt());

    Q_ASSERT(obj.contains("port_ranges"));
    QJsonValue portRanges = obj.take("port_ranges");
    Q_ASSERT(portRanges.isArray());

    QJsonArray portRangesArray = portRanges.toArray();
    for (QJsonValue portRangeValue : portRangesArray) {
        Q_ASSERT(portRangeValue.isArray());
        QJsonArray port = portRangeValue.toArray();
        Q_ASSERT(port.count() == 2);

        QJsonValue a = port.at(0);
        Q_ASSERT(a.isDouble());
        QJsonValue b = port.at(1);
        Q_ASSERT(b.isDouble());

        s.m_portRanges.append(QPair<uint32_t, uint32_t>(a.toInt(), b.toInt()));
    }

    return s;
}

// static
const Server &Server::weightChooser(const QList<Server> &servers)
{
    uint32_t weightSum = 0;

    for (const Server &server : servers) {
        weightSum += server.weight();
    }

    quint32 r = QRandomGenerator::global()->generate() % (weightSum + 1);

    for (const Server &server : servers) {
        if (server.weight() >= r) {
            return server;
        }

        r -= server.weight();
    }

    // This should not happen.
    Q_ASSERT(false);
    return servers[0];
}

uint32_t Server::choosePort() const
{
    quint32 r = QRandomGenerator::global()->generate() % m_portRanges.length();
    const QPair<uint32_t, uint32_t> &ports = m_portRanges.at(r);

    if (ports.first == ports.second) {
        return ports.first;
    }

    Q_ASSERT(ports.first < ports.second);
    return ports.first + (QRandomGenerator::global()->generate() % (ports.second - ports.first));
}
