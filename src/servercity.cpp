#include "servercity.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

ServerCity::ServerCity(const QString &name, const QString &code, double latitude, double longitude)
    : m_name(name), m_code(code), m_latitude(latitude), m_longitude(longitude)
{}

// static
ServerCity ServerCity::fromJson(QJsonObject &obj)
{
    qDebug() << obj.keys();

    Q_ASSERT(obj.contains("code"));
    QJsonValue code = obj.take("code");
    Q_ASSERT(code.isString());

    Q_ASSERT(obj.contains("latitude"));
    QJsonValue latitude = obj.take("latitude");
    Q_ASSERT(latitude.isDouble());

    Q_ASSERT(obj.contains("longitude"));
    QJsonValue longitude = obj.take("longitude");
    Q_ASSERT(longitude.isDouble());

    Q_ASSERT(obj.contains("name"));
    QJsonValue name = obj.take("name");
    Q_ASSERT(name.isString());

    ServerCity sc(name.toString(), code.toString(), latitude.toDouble(), longitude.toDouble());

    Q_ASSERT(obj.contains("servers"));
    QJsonValue servers = obj.take("servers");
    Q_ASSERT(servers.isArray());

    QJsonArray serversArray = servers.toArray();
    for (QJsonArray::Iterator i = serversArray.begin(); i != serversArray.end(); ++i) {
        Q_ASSERT(i->isObject());
        QJsonObject server = i->toObject();
        sc.m_servers.append(Server::fromJson(server));
    }

    qDebug() << "ServerCity" << sc.m_name << sc.m_code << sc.m_latitude << sc.m_longitude
             << "Servers:" << sc.m_servers.length();

    return sc;
}
