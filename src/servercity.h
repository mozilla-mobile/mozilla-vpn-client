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

    const QList<Server> getServers() const { return m_servers; }

private:
    ServerCity(const QString &name, const QString &code);

    QString m_name;
    QString m_code;

    QList<Server> m_servers;
};

#endif // SERVERCITY_H
