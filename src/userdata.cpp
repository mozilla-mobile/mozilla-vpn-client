#include "userdata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

// static
UserData UserData::fromJson(QJsonObject &obj)
{
    UserData data;

    Q_ASSERT(obj.contains("avatar"));
    QJsonValue avatarValue = obj.take("avatar");
    Q_ASSERT(avatarValue.isString());
    data.m_avatar = avatarValue.toString();

    Q_ASSERT(obj.contains("display_name"));
    QJsonValue displayName = obj.take("display_name");
    Q_ASSERT(displayName.isString());
    data.m_displayName = displayName.toString();

    Q_ASSERT(obj.contains("email"));
    QJsonValue email = obj.take("email");
    Q_ASSERT(email.isString());
    data.m_email = email.toString();

    Q_ASSERT(obj.contains("devices"));
    QJsonValue devices = obj.take("devices");
    Q_ASSERT(devices.isArray());
    QJsonArray devicesArray = devices.toArray();
    for (QJsonArray::iterator i = devicesArray.begin(); i != devicesArray.end(); ++i) {
        Q_ASSERT(i->isString());
        data.m_devices.append(i->toString());
    }

    Q_ASSERT(obj.contains("max_devices"));
    QJsonValue maxDevices = obj.take("max_devices");
    Q_ASSERT(maxDevices.isDouble());
    data.m_maxDevices = maxDevices.toInt();

    return data;
}

void UserData::writeSettings(QSettings &settings)
{
    settings.setValue("user/avatar", m_avatar);
    settings.setValue("user/displayName", m_displayName);
    settings.setValue("user/email", m_email);
    settings.setValue("user/devices", m_devices);
    settings.setValue("user/maxDevices", m_maxDevices);
}
