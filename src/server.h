#ifndef SERVER_H
#define SERVER_H

#include <QList>
#include <QPair>
#include <QString>

class QJsonObject;

class Server final
{
public:
    static Server fromJson(QJsonObject &obj);

private:
    Server(const QString &hostname,
           bool includeInCountry,
           const QString &ipv4AddrIn,
           const QString &ipv4Gateway,
           const QString &ipv6AddrIn,
           const QString &ipv6Gateway,
           const QString &publicKey,
           int weight)
        : m_hostname(hostname), m_includeInCountry(includeInCountry), m_ipv4AddrIn(ipv4AddrIn),
          m_ipv4Gateway(ipv4Gateway), m_ipv6AddrIn(ipv6AddrIn), m_ipv6Gateway(ipv6Gateway),
          m_publicKey(publicKey), m_weight(weight)
    {}

    QString m_hostname;
    bool m_includeInCountry;
    QString m_ipv4AddrIn;
    QString m_ipv4Gateway;
    QString m_ipv6AddrIn;
    QString m_ipv6Gateway;
    QList<QPair<uint32_t, uint32_t>> m_portRanges;
    QString m_publicKey;
    uint32_t m_weight;
};

#endif // SERVER_H
