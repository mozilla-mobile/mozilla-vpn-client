#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QJsonObject;
class QSettings;

class User final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString avatar READ avatar)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QString email READ email)
    Q_PROPERTY(int maxDevices READ maxDevices)

public:
    void fromJson(QJsonObject &obj);

    bool fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    const QString &avatar() const { return m_avatar; }

    const QString &displayName() const { return m_displayName; }

    const QString &email() const { return m_email; }

    // "Int"to make QML happy
    int maxDevices() const { return (int) m_maxDevices; }

private:
    QString m_avatar;
    QString m_displayName;
    QString m_email;
    uint32_t m_maxDevices;
};

#endif // USER_H
