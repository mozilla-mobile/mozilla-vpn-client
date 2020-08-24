#include "device.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

QString Device::currentDeviceName()
{
    QString deviceName = QSysInfo::machineHostName() + " " + QSysInfo::productType() + " "
                         + QSysInfo::productVersion();
    return deviceName;
}

Device Device::fromJson(const QJsonValue &json)
{
    Q_ASSERT(json.isObject());
    QJsonObject obj = json.toObject();

    Q_ASSERT(obj.contains("name"));
    QJsonValue name = obj.take("name");
    Q_ASSERT(name.isString());

    Q_ASSERT(obj.contains("pubkey"));
    QJsonValue pubKey = obj.take("pubkey");
    Q_ASSERT(pubKey.isString());

    Q_ASSERT(obj.contains("created_at"));
    QJsonValue createdAt = obj.take("created_at");
    Q_ASSERT(createdAt.isString());

    QDateTime date = QDateTime::fromString(createdAt.toString(), Qt::ISODate);
    Q_ASSERT(date.isValid());

    // No private key from JSON.

    return Device(name.toString(), date, pubKey.toString(), QString());
}

QList<Device> Device::fromSettings(QSettings &settings)
{
    QList<Device> list;

    if (settings.contains("devices")) {
        QStringList devices = settings.value("devices").toStringList();

        for (QStringList::Iterator i = devices.begin(); i != devices.end(); ++i) {
            QString key = "device/";
            key.append(*i);

            QString createdAt = key;
            createdAt.append("/createdAt");
            QString privkey = key;
            privkey.append("/privatekey");
            QString publickey = key;
            publickey.append("/publickey");

            list.append(Device(*i,
                               settings.value(createdAt).toDateTime(),
                               settings.value(publickey).toString(),
                               settings.value(privkey).toString()));
        }
    }

    return list;
}

void Device::writeSettings(QSettings &settings)
{
    settings.setValue("device/" + m_deviceName + "/createdAt", m_createdAt);
    settings.setValue("device/" + m_deviceName + "/privatekey", m_privateKey);
    settings.setValue("device/" + m_deviceName + "/publickey", m_publicKey);
}
