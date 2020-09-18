#include "device.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

#ifdef QT_DEBUG
#include <QRandomGenerator>
#endif

#ifdef IOS_INTEGRATION
#include "platforms/ios/iosutils.h"
#elif __APPLE__
#include "platforms/macos/macosutils.h"
#endif // __APPLE__

QString Device::currentDeviceName()
{
    QString deviceName =

#ifdef IOS_INTEGRATION
	IOSUtils::computerName();
#elif __APPLE__
        // MacOS has a funny way to rename the hostname based on the network status.
        MacOSUtils::computerName();
#else
        QSysInfo::machineHostName() + " " + QSysInfo::productType() + " "
        + QSysInfo::productVersion();
#endif

    /*  If we want to generate a new device name at each execution, comment out this block:
#ifdef QT_DEBUG
    static quint32 uniqueId = 0;
    if (uniqueId == 0) {
        uniqueId = QRandomGenerator::global()->generate();
    }

    deviceName = QString("%1 %2").arg(deviceName).arg(uniqueId);
#endif
    */

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

    Q_ASSERT(obj.contains("ipv4_address"));
    QJsonValue ipv4Address = obj.take("ipv4_address");
    Q_ASSERT(ipv4Address.isString());

    Q_ASSERT(obj.contains("ipv6_address"));
    QJsonValue ipv6Address = obj.take("ipv6_address");
    Q_ASSERT(ipv6Address.isString());

    QDateTime date = QDateTime::fromString(createdAt.toString(), Qt::ISODate);
    Q_ASSERT(date.isValid());

    return Device(name.toString(),
                  date,
                  pubKey.toString(),
                  ipv4Address.toString(),
                  ipv6Address.toString());
}
