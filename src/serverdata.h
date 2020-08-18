#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QByteArray>
#include <QObject>

class QSettings;

class ServerData : public QObject
{
public:
    static ServerData *fromSettings(QSettings &settings);

    static ServerData *fromJson(QByteArray &data);

    void writeSettings(QSettings &settings);

private:
    ServerData() = default;

    void fromJsonInternal();

private:
    QByteArray m_rawJson;

    struct Country
    {
        Country(const QString &name, const QString &code) : m_name(name), m_code(code) {}

        QString m_name;
        QString m_code;
    };

    QList<Country> m_countries;
};

#endif // SERVERDATA_H
