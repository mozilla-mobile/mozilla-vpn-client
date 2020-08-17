#ifndef USERDATA_H
#define USERDATA_H

#include <QString>
#include <QStringList>

class QJsonObject;
class QSettings;

class UserData final
{
public:
    static UserData fromJson(QJsonObject &obj);

    void writeSettings(QSettings &settings);

    bool hasDevice(const QString &deviceName) const { return m_devices.contains(deviceName); }

private:
    UserData() = default;

private:
    QString m_avatar;
    QString m_displayName;
    QString m_email;
    QStringList m_devices;
    uint32_t m_maxDevices;
};

#endif // USERDATA_H
