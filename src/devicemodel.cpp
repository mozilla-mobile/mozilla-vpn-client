#include "devicemodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

void DeviceModel::fromJson(QJsonObject &obj)
{
    qDebug() << "DeviceModel from json";

    QString privateKey;
    QString currentDeviceName = Device::currentDeviceName();
    if (hasPrivateKeyDevice(currentDeviceName)) {
        privateKey = device(currentDeviceName)->privateKey();
    }

    beginResetModel();

    m_devices.clear();

    Q_ASSERT(obj.contains("devices"));
    QJsonValue devices = obj.take("devices");
    Q_ASSERT(devices.isArray());
    QJsonArray devicesArray = devices.toArray();
    for (QJsonArray::iterator i = devicesArray.begin(); i != devicesArray.end(); ++i) {
        Device device = Device::fromJson(*i);

        if (device.name() == currentDeviceName) {
            device.setPrivateKey(privateKey);
        }

        m_devices.append(device);
    }

    endResetModel();
    emit changed();
}

bool DeviceModel::fromSettings(QSettings &settings)
{
    m_devices = Device::fromSettings(settings);
    return !m_devices.isEmpty();
}

void DeviceModel::writeSettings(QSettings &settings)
{
    QStringList keys = settings.allKeys();
    for (QStringList::Iterator i = keys.begin(); i != keys.end(); ++i) {
        if (i->startsWith("device/")) {
            settings.remove(*i);
        }
    }

    for (QList<Device>::Iterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        i->writeSettings(settings);
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CurrentOneRole] = "currentOne";
    return roles;
}

int DeviceModel::rowCount(const QModelIndex &) const
{
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return QVariant(m_devices.at(index.row()).name());

    case CurrentOneRole:
        return QVariant(m_devices.at(index.row()).hasPrivateKey());

    default:
        return QVariant();
    }
}

bool DeviceModel::hasDevice(const QString &deviceName) const
{
    for (QList<Device>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName)) {
            return true;
        }
    }

    return false;
}

bool DeviceModel::hasPrivateKeyDevice(const QString &deviceName) const
{
    for (QList<Device>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName) && i->hasPrivateKey()) {
            return true;
        }
    }

    return false;
}

const Device *DeviceModel::device(const QString &deviceName) const
{
    for (QList<Device>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName)) {
            return &(*i);
        }
    }

    return nullptr;
}

void DeviceModel::addDevice(const Device &device)
{
    m_devices.append(device);
}

void DeviceModel::removeDevice(const QString &deviceName)
{
    // TODO: we can be smarter here and remove the single item.
    beginResetModel();

    QMutableListIterator<Device> i(m_devices);
    while (i.hasNext()) {
        const Device &device = i.next();
        if (device.isDevice(deviceName)) {
            i.remove();
            break;
        }
    }

    endResetModel();
    emit changed();
}
