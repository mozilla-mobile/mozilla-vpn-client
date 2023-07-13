/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "devicemodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#if MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif

namespace {
Logger logger("DeviceModel");
}

DeviceModel::DeviceModel() {
  MZ_COUNT_CTOR(DeviceModel);
  LogHandler::instance()->registerLogSerializer(this);
}

DeviceModel::~DeviceModel() {
  MZ_COUNT_DTOR(DeviceModel);
  LogHandler::instance()->unregisterLogSerializer(this);
}

bool DeviceModel::fromJson(const Keys* keys, const QByteArray& s) {
  logger.debug() << "DeviceModel from json";

  if (!s.isEmpty() && m_rawJson == s) {
    logger.debug() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(keys, s)) {
    return false;
  }

  m_rawJson = s;
  return true;
}

bool DeviceModel::fromSettings(const Keys* keys) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading the device list from settings";

  const QByteArray& json = settingsHolder->devices();
  if (json.isEmpty() || !fromJsonInternal(keys, json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

namespace {

bool sortCallback(const Device& a, const Device& b, const Keys* keys) {
  if (a.isCurrentDevice(keys)) {
    return true;
  }

  if (b.isCurrentDevice(keys)) {
    return false;
  }

  return a.createdAt() > b.createdAt();
}

}  // anonymous namespace

bool DeviceModel::fromJsonInternal(const Keys* keys, const QByteArray& json) {
  beginResetModel();

  // Maybe we have to refresh the device list during a removal operation. If
  // this happens, maybe we have to store some of the "incoming" devices in the
  // list of the removed ones.
  // This is done comparing the list of the publicKeys of the removed devices
  // with the new ones.
  QStringList removedPublicKeys;
  for (const Device& removedDevice : m_removedDevices) {
    removedPublicKeys.append(removedDevice.publicKey());
  }

  m_rawJson = "";
  m_devices.clear();
  m_removedDevices.clear();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  if (!obj.contains("devices")) {
    return false;
  }

  QJsonValue devices = obj.value("devices");
  if (!devices.isArray()) {
    return false;
  }

  const QJsonArray devicesArray = devices.toArray();
  for (const QJsonValue& deviceValue : devicesArray) {
    Device device;
    if (!device.fromJson(deviceValue)) {
      return false;
    }

    if (removedPublicKeys.contains(device.publicKey())) {
      m_removedDevices.append(device);
    } else {
      m_devices.append(device);
    }
  }

  std::sort(m_devices.begin(), m_devices.end(),
            std::bind(sortCallback, std::placeholders::_1,
                      std::placeholders::_2, keys));

  endResetModel();
  emit changed();

  return true;
}

void DeviceModel::writeSettings() {
  SettingsHolder::instance()->setDevices(m_rawJson);
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[PublicKeyRole] = "publicKey";
  roles[CurrentOneRole] = "currentOne";
  roles[CreatedAtRole] = "createdAt";
  return roles;
}

int DeviceModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(m_devices.count());
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case NameRole:
      return QVariant(m_devices.at(index.row()).name());

    case PublicKeyRole:
      return QVariant(m_devices.at(index.row()).publicKey());

    case CurrentOneRole:
      return QVariant(m_devices.at(index.row())
                          .isCurrentDevice(MozillaVPN::instance()->keys()));

    case CreatedAtRole:
      return QVariant(m_devices.at(index.row()).createdAt());

    default:
      return QVariant();
  }
}

void DeviceModel::startDeviceRemovalFromPublicKey(const QString& publicKey) {
  for (int i = 0; i < m_devices.length(); ++i) {
    if (m_devices.at(i).publicKey() == publicKey) {
      // Let's remove this device in a separate list to restore it in case the
      // removal fails or in case we have to refresh the whole model.
      m_removedDevices.append(m_devices.at(i));
      removeRow(i);
      emit changed();
      return;
    }
  }
}

void DeviceModel::stopDeviceRemovalFromPublicKey(const QString& publicKey,
                                                 const Keys* keys) {
  for (auto i = m_removedDevices.begin(); i != m_removedDevices.end(); ++i) {
    if (i->publicKey() == publicKey) {
      // We were not supposed to find the device in this list. If this happens
      // is because something went wrong during the removal operation. Let's
      // bring the device back.
      beginResetModel();

      m_devices.append(*i);

      std::sort(m_devices.begin(), m_devices.end(),
                std::bind(sortCallback, std::placeholders::_1,
                          std::placeholders::_2, keys));

      m_removedDevices.erase(i);

      endResetModel();
      emit changed();
      break;
    }
  }
}

void DeviceModel::removeDeviceFromPublicKey(const QString& publicKey) {
  for (auto i = m_removedDevices.begin(); i != m_removedDevices.end(); ++i) {
    if (i->publicKey() == publicKey) {
      m_removedDevices.erase(i);
      break;
    }
  }

  for (int i = 0; i < m_devices.length(); ++i) {
    if (m_devices.at(i).publicKey() == publicKey) {
      removeRow(i);
      emit changed();
      return;
    }
  }
}

bool DeviceModel::removeRows(int row, int count, const QModelIndex& parent) {
  Q_ASSERT(count == 1);
  Q_UNUSED(parent);

  beginRemoveRows(parent, row, row);
  m_devices.removeAt(row);
  endRemoveRows();

  return true;
}

const Device* DeviceModel::currentDevice(const Keys* keys) const {
  for (const Device& device : m_devices) {
    if (device.isCurrentDevice(keys)) {
      return &device;
    }
  }

  return nullptr;
}

const Device* DeviceModel::deviceFromPublicKey(const QString& publicKey) const {
  for (const Device& device : m_devices) {
    if (device.publicKey() == publicKey) {
      return &device;
    }
  }

  return nullptr;
}

const Device* DeviceModel::deviceFromUniqueId() const {
  QString uniqueId = Device::uniqueDeviceId();
  if (uniqueId.isEmpty()) {
    return nullptr;
  }

  for (const Device& device : m_devices) {
    if (device.uniqueId() == uniqueId) {
      return &device;
    }
  }

  return nullptr;
}

bool DeviceModel::hasCurrentDevice(const Keys* keys) const {
  return currentDevice(keys) != nullptr;
}

void DeviceModel::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString& logs)> callback =
      std::move(a_callback);

  QString buffer;
  QTextStream out(&buffer);
  out << "ABI -> " << QSysInfo::buildAbi() << Qt::endl;
  out << "Machine arch -> " << QSysInfo::currentCpuArchitecture() << Qt::endl;
  out << "OS -> " << QSysInfo::productType() << Qt::endl;
#ifdef MZ_WINDOWS
  out << "OS Version -> " << WindowsUtils::windowsVersion() << Qt::endl;
#else
  out << "OS Version -> " << QSysInfo::productVersion() << Qt::endl;
#endif
#ifdef MZ_ANDROID
  out << "SDK Version -> " << AndroidCommons::getSDKVersion() << Qt::endl;
#endif

  out << "APP Version -> " << AppConstants::versionString() << Qt::endl;
  out << "Build ID -> " << AppConstants::buildNumber() << Qt::endl;
  out << "Device ID -> " << Device::uniqueDeviceId() << Qt::endl;

#ifndef QT_NO_SSL
  out << "SSL Lib:" << QSslSocket::sslLibraryVersionString()
      << QSslSocket::sslLibraryVersionNumber() << Qt::endl;
#endif

  callback("Device", buffer);
}
