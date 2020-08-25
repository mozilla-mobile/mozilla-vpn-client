#include "user.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutableListIterator>
#include <QSettings>

constexpr const char *SETTINGS_AVATAR = "user/avatar";
constexpr const char *SETTINGS_DISPLAYNAME = "user/displayName";
constexpr const char *SETTINGS_EMAIL = "user/email";
constexpr const char *SETTINGS_MAXDEVICES = "user/maxDevices";

void User::fromJson(const QByteArray &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);

    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();

    Q_ASSERT(obj.contains("avatar"));
    QJsonValue avatarValue = obj.take("avatar");
    Q_ASSERT(avatarValue.isString());
    m_avatar = avatarValue.toString();

    Q_ASSERT(obj.contains("display_name"));
    QJsonValue displayName = obj.take("display_name");
    Q_ASSERT(displayName.isString());
    m_displayName = displayName.toString();

    Q_ASSERT(obj.contains("email"));
    QJsonValue email = obj.take("email");
    Q_ASSERT(email.isString());
    m_email = email.toString();

    Q_ASSERT(obj.contains("max_devices"));
    QJsonValue maxDevices = obj.take("max_devices");
    Q_ASSERT(maxDevices.isDouble());
    m_maxDevices = maxDevices.toInt();

    emit changed();
}

bool User::fromSettings(QSettings &settings)
{
    if (!settings.contains(SETTINGS_AVATAR) || !settings.contains(SETTINGS_DISPLAYNAME)
        || !settings.contains(SETTINGS_EMAIL) || !settings.contains(SETTINGS_MAXDEVICES)) {
        return false;
    }

    m_avatar = settings.value(SETTINGS_AVATAR).toString();
    m_displayName = settings.value(SETTINGS_DISPLAYNAME).toString();
    m_email = settings.value(SETTINGS_EMAIL).toString();
    m_maxDevices = settings.value(SETTINGS_MAXDEVICES).toUInt();

    return true;
}

void User::writeSettings(QSettings &settings)
{
    settings.setValue(SETTINGS_AVATAR, m_avatar);
    settings.setValue(SETTINGS_DISPLAYNAME, m_displayName);
    settings.setValue(SETTINGS_EMAIL, m_email);
    settings.setValue(SETTINGS_MAXDEVICES, m_maxDevices);
}
