/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "devicemodel.h"
#include "logger.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger("DeviceModel");
}

void DeviceModel::fromJson(const QByteArray &s)
{
    logger.log() << "DeviceModel from json";

    if (m_rawJson == s) {
        logger.log() << "Nothing has changed";
        return;
    }

    m_rawJson = s;
    fromJsonInternal();
}

bool DeviceModel::fromSettings(SettingsHolder &settingsHolder)
{
    logger.log() << "Reading the device list from settings";

    if (!settingsHolder.hasDevices()) {
        return false;
    }

    m_rawJson = settingsHolder.devices();
    if (!fromJsonInternal()) {
        return false;
    }

    return !m_devices.isEmpty();
}

namespace {

bool sortCallback(const Device &a, const Device &b)
{
    if (a.name() == Device::currentDeviceName()) {
        return true;
    }

    if (b.name() == Device::currentDeviceName()) {
        return false;
    }

    return a.createdAt() > b.createdAt();
}

} // anonymous namespace

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

    std::sort(m_devices.begin(), m_devices.end(), sortCallback);

    endResetModel();
    emit changed();

    return true;
}

void DeviceModel::writeSettings(SettingsHolder &settingsHolder)
{
    settingsHolder.setDevices(m_rawJson);
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
