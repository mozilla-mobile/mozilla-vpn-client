#ifndef SERVERCOUNTRY_H
#define SERVERCOUNTRY_H

#include "servercity.h"

#include <QList>
#include <QString>

class QJsonObject;
class QStringList;

class ServerCountry final
{
public:
    static ServerCountry fromJson(QJsonObject &obj);

    const QString &name() const { return m_name; }

    const QString &code() const { return m_code; }

    const QList<ServerCity> &cities() const { return m_cities; }

private:
    ServerCountry(const QString &name, const QString &code) : m_name(name), m_code(code) {}

    QString m_name;
    QString m_code;

    QList<ServerCity> m_cities;
};

#endif // SERVERCOUNTRY_H
