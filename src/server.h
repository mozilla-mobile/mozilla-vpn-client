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

    static const Server &weightChooser(const QList<Server> &servers);

    const QString &hostname() const { return m_hostname; }

    const QString& ipv4AddrIn() const { return m_ipv4AddrIn; }

    const QString& ipv4Gateway() const { return m_ipv4Gateway; }

    const QString& ipv6AddrIn() const { return m_ipv6AddrIn; }

    const QString& ipv6Gateway() const { return m_ipv6Gateway; }

    const QString& publicKey() const { return m_publicKey; }

    uint32_t weight() const { return m_weight; }

    uint32_t choosePort() const;

private:
    Server() = default;

    Server(const QString &hostname,
           const QString &ipv4AddrIn,
           const QString &ipv4Gateway,
           const QString &ipv6AddrIn,
           const QString &ipv6Gateway,
           const QString &publicKey,
           uint32_t weight)
        : m_hostname(hostname), m_ipv4AddrIn(ipv4AddrIn), m_ipv4Gateway(ipv4Gateway),
          m_ipv6AddrIn(ipv6AddrIn), m_ipv6Gateway(ipv6Gateway), m_publicKey(publicKey),
          m_weight(weight)
    {}

    QString m_hostname;
    QString m_ipv4AddrIn;
    QString m_ipv4Gateway;
    QString m_ipv6AddrIn;
    QString m_ipv6Gateway;
    QList<QPair<uint32_t, uint32_t>> m_portRanges;
    QString m_publicKey;
    uint32_t m_weight;
};

#endif // SERVER_H
