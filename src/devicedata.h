#ifndef DEVICEDATA_H
#define DEVICEDATA_H

#include <QList>

class QSettings;
class QJsonValue;

class DeviceData
{
public:
    DeviceData(const QString &deviceName, const QString &publicKey)
        : m_deviceName(deviceName), m_publicKey(publicKey)
    {}

    static DeviceData fromJson(const QJsonValue &json);

    static QList<DeviceData> fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    bool isDevice(const QString &deviceName) const { return m_deviceName == deviceName; }

private:
    QString m_deviceName;
    QString m_publicKey;
};

#endif // DEVICEDATA_H
