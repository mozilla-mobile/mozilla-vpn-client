/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCITY_H
#define SERVERCITY_H

#include "server.h"

#include <QList>
#include <QString>

class QJsonObject;

class ServerCity final
{
public:
    static ServerCity fromJson(QJsonObject &obj);

    const QString &name() const { return m_name; }

    const QString &code() const { return m_code; }

    const QList<Server> getServers() const { return m_servers; }

private:
    ServerCity(const QString &name, const QString &code);

    QString m_name;
    QString m_code;

    QList<Server> m_servers;
};

#endif // SERVERCITY_H
