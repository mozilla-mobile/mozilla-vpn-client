#include "server.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

// static
Server Server::fromJson(QJsonObject &obj)
{
    Q_ASSERT(obj.contains("hostname"));
    QJsonValue hostname = obj.take("hostname");
    Q_ASSERT(hostname.isString());

    Q_ASSERT(obj.contains("include_in_country"));
    QJsonValue includeInCountry = obj.take("include_in_country");
    Q_ASSERT(includeInCountry.isBool());

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
             includeInCountry.toBool(),
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

    qDebug() << "Server: " << s.m_hostname << s.m_includeInCountry << s.m_ipv4AddrIn
             << s.m_ipv4Gateway << s.m_ipv6AddrIn << s.m_ipv6Gateway << s.m_publicKey << s.m_weight
             << s.m_portRanges;
    return s;
}
