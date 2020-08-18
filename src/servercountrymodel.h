#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include <QByteArray>
#include <QObject>

class QSettings;

class ServerCountryModel : public QObject
{
public:
    ServerCountryModel() = default;

    bool fromSettings(QSettings &settings);

    void fromJson(const QByteArray &data);

    void writeSettings(QSettings &settings);

private:
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

#endif // SERVERCOUNTRYMODEL_H
