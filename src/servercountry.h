#ifndef SERVERCOUNTRY_H
#define SERVERCOUNTRY_H

#include <QObject>

class ServerCountry : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString code READ code)

public:
    ServerCountry(const QString &name, const QString &code) : m_name(name), m_code(code) {}

    const QString &name() const { return m_name; }

    const QString &code() const { return m_code; }

private:
    QString m_name;
    QString m_code;

signals:

};

#endif // SERVERCOUNTRY_H
