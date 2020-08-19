#ifndef SERVERCITY_H
#define SERVERCITY_H

#include "server.h"

#include <QList>
#include <QString>

class QJsonObject;

class ServerCity
{
public:
    static ServerCity fromJson(QJsonObject &obj);

    const QString &name() const { return m_name; }

private:
    ServerCity(const QString &name, const QString &code, double latitude, double longitude);

    QString m_name;
    QString m_code;
    double m_latitude;
    double m_longitude;

    QList<Server> m_servers;
};

#endif // SERVERCITY_H
