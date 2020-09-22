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

    Q_PROPERTY(QString avatar READ avatar NOTIFY changed)
    Q_PROPERTY(QString displayName READ displayName NOTIFY changed)
    Q_PROPERTY(QString email READ email NOTIFY changed)
    Q_PROPERTY(int maxDevices READ maxDevices NOTIFY changed)

public:
    void fromJson(const QByteArray &json);

    bool fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    const QString &avatar() const { return m_avatar; }

    const QString &displayName() const { return m_displayName; }

    const QString &email() const { return m_email; }

    // "Int"to make QML happy
    int maxDevices() const { return (int) m_maxDevices; }

    bool subscriptionNeeded() const { return m_subscriptionNeeded; }

signals:
    void changed();

private:
    QString m_avatar;
    QString m_displayName;
    QString m_email;
    uint32_t m_maxDevices = 5;
    bool m_subscriptionNeeded = false;
};

#endif // USER_H
