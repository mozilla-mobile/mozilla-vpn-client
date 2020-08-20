#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QJsonObject;
class QSettings;

class User final
{
public:
    void fromJson(QJsonObject &obj);

    bool fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    uint32_t maxDevices() const { return m_maxDevices; }

private:
    QString m_avatar;
    QString m_displayName;
    QString m_email;
    uint32_t m_maxDevices;
};

#endif // USER_H
