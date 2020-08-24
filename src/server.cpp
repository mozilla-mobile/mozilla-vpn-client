#include "server.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

// static
Server Server::fromJson(QJsonObject &obj)
{
    Q_ASSERT(obj.contains("hostname"));
    QJsonValue hostname = obj.take("hostname");
    Q_ASSERT(hostname.isString());

    Q_ASSERT(obj.contains("ipv4_addr_in"));
    QJsonValue ipv4AddrIn = obj.take("ipv4_addr_in");
    Q_ASSERT(ipv4AddrIn.isString());

    Q_ASSERT(obj.contains("ipv4_gateway"));
    QJsonValue ipv4Gateway = obj.take("ipv4_gateway");
    Q_ASSERT(ipv4Gateway.isString());

    Q_ASSERT(obj.contains("ipv6_addr_in"));
    QJsonValue ipv6AddrIn = obj.take("ipv6_addr_in");
    Q_ASSERT(ipv6AddrIn.isString());

    Q_ASSERT(obj.contains("ipv6_gateway"));
    QJsonValue ipv6Gateway = obj.take("ipv6_gateway");
    Q_ASSERT(ipv6Gateway.isString());

    Q_ASSERT(obj.contains("public_key"));
    QJsonValue publicKey = obj.take("public_key");
    Q_ASSERT(publicKey.isString());

    Q_ASSERT(obj.contains("weight"));
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
    for (QJsonArray::Iterator i = portRangesArray.begin(); i != portRangesArray.end(); ++i) {
        Q_ASSERT(i->isArray());
        QJsonArray port = i->toArray();
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

    for (QList<Server>::ConstIterator i = servers.begin(); i != servers.end(); ++i) {
        weightSum += i->weight();
    }

    quint32 r = QRandomGenerator::global()->generate() % (weightSum + 1);

    for (QList<Server>::ConstIterator i = servers.begin(); i != servers.end(); ++i) {
        if (i->weight() <= r) {
            return *i;
        }

        r -= i->weight();
    }

    // This should not happen.
    Q_ASSERT(false);
    return servers[0];
}
