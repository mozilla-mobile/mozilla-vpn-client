#ifndef DEVICEDATA_H
#define DEVICEDATA_H

#include <QList>

class QSettings;
class QJsonValue;

class DeviceData
{
public:
    DeviceData(const QString &deviceName, const QString &publicKey, const QString &privateKey)
        : m_deviceName(deviceName), m_publicKey(publicKey), m_privateKey(privateKey)
    {}

    static QString currentDeviceName();

    static DeviceData fromJson(const QJsonValue &json);

    static QList<DeviceData> fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    const QString &deviceName() const { return m_deviceName; }

    bool isDevice(const QString &deviceName) const { return m_deviceName == deviceName; }

    bool hasPrivateKey() const { return !m_privateKey.isEmpty(); }

    const QString &publicKey() const { return m_publicKey; }

private:
    QString m_deviceName;
    QString m_publicKey;
    QString m_privateKey;
};

#endif // DEVICEDATA_H
