#include "devicedata.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

DeviceData DeviceData::fromJson(const QJsonValue &json)
{
    Q_ASSERT(json.isObject());
    QJsonObject obj = json.toObject();

    Q_ASSERT(obj.contains("name"));
    QJsonValue name = obj.take("name");
    Q_ASSERT(name.isString());

    Q_ASSERT(obj.contains("pubkey"));
    QJsonValue pubKey = obj.take("pubkey");
    Q_ASSERT(pubKey.isString());

    return DeviceData(name.toString(), pubKey.toString());
}

QList<DeviceData> DeviceData::fromSettings(QSettings &settings)
{
    QList<DeviceData> list;

    QStringList keys = settings.allKeys();
    for (QStringList::Iterator i = keys.begin(); i != keys.end(); ++i) {
        if (i->startsWith("device/")) {
            QString deviceName = *i;
            deviceName.remove(0, 7);

            QString pubKey = settings.value(*i).toString();
            list.append(DeviceData(deviceName, pubKey));
        }
    }

    return list;
}

void DeviceData::writeSettings(QSettings &settings)
{
    settings.setValue("device/" + m_deviceName, m_publicKey);
}
