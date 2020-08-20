#ifndef DEVICE_H
#define DEVICE_H

#include <QList>

class QSettings;
class QJsonValue;

class Device final
{
public:
    Device(const QString &deviceName, const QString &publicKey, const QString &privateKey)
        : m_deviceName(deviceName), m_publicKey(publicKey), m_privateKey(privateKey)
    {}

    static QString currentDeviceName();

    static Device fromJson(const QJsonValue &json);

    static QList<Device> fromSettings(QSettings &settings);

    void writeSettings(QSettings &settings);

    const QString &name() const { return m_deviceName; }

    bool isDevice(const QString &deviceName) const { return m_deviceName == deviceName; }

    bool hasPrivateKey() const { return !m_privateKey.isEmpty(); }

    const QString &privateKey() const { return m_privateKey; }

    void setPrivateKey(const QString &privateKey) { m_privateKey = privateKey; }

    const QString &publicKey() const { return m_publicKey; }

private:
    QString m_deviceName;
    QString m_publicKey;
    QString m_privateKey;
};

#endif // DEVICE_H
