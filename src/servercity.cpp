/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercity.h"

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
    for (QJsonValue serverValue : serversArray) {
        Q_ASSERT(serverValue.isObject());
        QJsonObject server = serverValue.toObject();
        sc.m_servers.append(Server::fromJson(server));
    }

    return sc;
}
