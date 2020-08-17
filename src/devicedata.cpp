#include "devicedata.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

QString DeviceData::currentDeviceName()
{
    QString deviceName = QSysInfo::machineHostName() + " " + QSysInfo::productType() + " "
                         + QSysInfo::productVersion();
    return deviceName;
}

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

    // No private key from JSON.

    return DeviceData(name.toString(), pubKey.toString(), QString());
}

QList<DeviceData> DeviceData::fromSettings(QSettings &settings)
{
    QList<DeviceData> list;

    QStringList keys = settings.allKeys();
    for (QStringList::Iterator i = keys.begin(); i != keys.end(); ++i) {
        if (i->startsWith("device/")) {
            QString deviceName = *i;
            deviceName.remove(0, 7);

            QStringList keys = settings.value(*i).toStringList();
            Q_ASSERT(keys.length() >= 1);
            list.append(DeviceData(deviceName, keys[0], keys.count() > 1 ? keys[1] : QString()));
        }
    }

    return list;
}

void DeviceData::writeSettings(QSettings &settings)
{
    QStringList keys;
    keys.append(m_publicKey);
    keys.append(m_privateKey);
    settings.setValue("device/" + m_deviceName, keys);
}
