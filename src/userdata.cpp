#include "userdata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutableListIterator>
#include <QSettings>

constexpr const char *SETTINGS_AVATAR = "user/avatar";
constexpr const char *SETTINGS_DISPLAYNAME = "user/displayName";
constexpr const char *SETTINGS_EMAIL = "user/email";
constexpr const char *SETTINGS_MAXDEVICES = "user/maxDevices";

// static
UserData *UserData::fromJson(QJsonObject &obj)
{
    UserData *data = new UserData();

    Q_ASSERT(obj.contains("avatar"));
    QJsonValue avatarValue = obj.take("avatar");
    Q_ASSERT(avatarValue.isString());
    data->m_avatar = avatarValue.toString();

    Q_ASSERT(obj.contains("display_name"));
    QJsonValue displayName = obj.take("display_name");
    Q_ASSERT(displayName.isString());
    data->m_displayName = displayName.toString();

    Q_ASSERT(obj.contains("email"));
    QJsonValue email = obj.take("email");
    Q_ASSERT(email.isString());
    data->m_email = email.toString();

    Q_ASSERT(obj.contains("devices"));
    QJsonValue devices = obj.take("devices");
    Q_ASSERT(devices.isArray());
    QJsonArray devicesArray = devices.toArray();
    for (QJsonArray::iterator i = devicesArray.begin(); i != devicesArray.end(); ++i) {
        data->m_devices.append(DeviceData::fromJson(*i));
    }

    Q_ASSERT(obj.contains("max_devices"));
    QJsonValue maxDevices = obj.take("max_devices");
    Q_ASSERT(maxDevices.isDouble());
    data->m_maxDevices = maxDevices.toInt();

    return data;
}

// static
UserData *UserData::fromSettings(QSettings &settings)
{
    if (!settings.contains(SETTINGS_AVATAR) || !settings.contains(SETTINGS_DISPLAYNAME)
        || !settings.contains(SETTINGS_EMAIL) || !settings.contains(SETTINGS_MAXDEVICES)) {
        return nullptr;
    }

    UserData *data = new UserData();
    data->m_avatar = settings.value(SETTINGS_AVATAR).toString();
    data->m_displayName = settings.value(SETTINGS_DISPLAYNAME).toString();
    data->m_email = settings.value(SETTINGS_EMAIL).toString();
    data->m_maxDevices = settings.value(SETTINGS_MAXDEVICES).toUInt();
    data->m_devices = DeviceData::fromSettings(settings);

    return data;
}

void UserData::writeSettings(QSettings &settings)
{
    settings.setValue(SETTINGS_AVATAR, m_avatar);
    settings.setValue(SETTINGS_DISPLAYNAME, m_displayName);
    settings.setValue(SETTINGS_EMAIL, m_email);
    settings.setValue(SETTINGS_MAXDEVICES, m_maxDevices);

    for (QList<DeviceData>::Iterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        i->writeSettings(settings);
    }
}

bool UserData::hasDevice(const QString &deviceName) const
{
    for (QList<DeviceData>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName)) {
            return true;
        }
    }

    return false;
}

bool UserData::hasPrivateKeyDevice(const QString &deviceName) const
{
    for (QList<DeviceData>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName) && i->hasPrivateKey()) {
            return true;
        }
    }

    return false;
}

const DeviceData *UserData::device(const QString &deviceName) const
{
    for (QList<DeviceData>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName)) {
            return &(*i);
        }
    }

    return nullptr;
}

void UserData::addDevice(const DeviceData &deviceData)
{
    m_devices.append(deviceData);
}

void UserData::removeDevice(const QString &deviceName)
{
    QMutableListIterator<DeviceData> i(m_devices);
    while (i.hasNext()) {
        if (i.value().isDevice(deviceName)) {
            i.remove();
            break;
        }
    }
}
