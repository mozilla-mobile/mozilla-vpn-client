#include "servercity.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

ServerCity::ServerCity(const QString &name, const QString &code) : m_name(name), m_code(code) {}

// static
ServerCity ServerCity::fromJson(QJsonObject &obj)
{
    Q_ASSERT(obj.contains("code"));
    QJsonValue code = obj.take("code");
    Q_ASSERT(code.isString());

    Q_ASSERT(obj.contains("name"));
    QJsonValue name = obj.take("name");
    Q_ASSERT(name.isString());

    ServerCity sc(name.toString(), code.toString());

    Q_ASSERT(obj.contains("servers"));
    QJsonValue servers = obj.take("servers");
    Q_ASSERT(servers.isArray());

    QJsonArray serversArray = servers.toArray();
    for (QJsonArray::Iterator i = serversArray.begin(); i != serversArray.end(); ++i) {
        Q_ASSERT(i->isObject());
        QJsonObject server = i->toObject();
        sc.m_servers.append(Server::fromJson(server));
    }

    return sc;
}
