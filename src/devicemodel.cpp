#include "devicemodel.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

void DeviceModel::fromJson(const QByteArray& s)
{
    qDebug() << "DeviceModel from json";

    m_rawJson = s;
    fromJsonInternal();
}

bool DeviceModel::fromSettings(QSettings &settings)
{
    qDebug() << "Reading the device list from settings";

    if (!settings.contains("devices")) {
        return false;
    }

    m_rawJson = settings.value("devices").toByteArray();
    if (!fromJsonInternal()) {
        return false;
    }

    return !m_devices.isEmpty();
}

bool DeviceModel::fromJsonInternal() {
    beginResetModel();

    m_devices.clear();

    QJsonDocument doc = QJsonDocument::fromJson(m_rawJson);
    if (doc.isNull()) {
        return false;
    }

    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();

    Q_ASSERT(obj.contains("devices"));
    QJsonValue devices = obj.take("devices");
    Q_ASSERT(devices.isArray());
    QJsonArray devicesArray = devices.toArray();
    for (QJsonArray::iterator i = devicesArray.begin(); i != devicesArray.end(); ++i) {
        Device device = Device::fromJson(*i);
        m_devices.append(device);
    }

    endResetModel();
    emit changed();

    return true;
}

void DeviceModel::writeSettings(QSettings &settings)
{
    settings.setValue("devices", m_rawJson);
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CurrentOneRole] = "currentOne";
    roles[CreatedAtRole] = "createdAt";
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
        return QVariant(m_devices.at(index.row()).name() == Device::currentDeviceName());

    case CreatedAtRole:
        return QVariant(m_devices.at(index.row()).createdAt());

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

const Device *DeviceModel::device(const QString &deviceName) const
{
    for (QList<Device>::ConstIterator i = m_devices.begin(); i != m_devices.end(); ++i) {
        if (i->isDevice(deviceName)) {
            return &(*i);
        }
    }

    return nullptr;
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

const Device* DeviceModel::currentDevice() const {
    return device(Device::currentDeviceName());
}
