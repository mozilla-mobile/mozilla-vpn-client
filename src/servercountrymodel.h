#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include <QByteArray>
#include <QObject>
#include <QPointer>

class ServerCountry;
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

    QList<QPointer<ServerCountry>> m_countries;
};

#endif // SERVERCOUNTRYMODEL_H
