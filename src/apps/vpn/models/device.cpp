/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "device.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>

#include "constants.h"
#include "keys.h"
#include "leakdetector.h"

#ifndef QT_NO_SSL
#  include <QSslSocket>
#endif

#ifdef MZ_DEBUG
#  include <QRandomGenerator>
#endif

#ifdef MZ_IOS
#  include "platforms/ios/iosutils.h"
#elif MZ_MACOS
#  include "platforms/macos/macosutils.h"
#elif MZ_ANDROID
#  include "platforms/android/androidutils.h"
#elif MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif

// static
QString Device::currentDeviceName() {
  QString deviceName =

#ifdef MZ_IOS
      IOSUtils::computerName();
#elif MZ_MACOS
      // MacOS has a funny way to rename the hostname based on the network
      // status.
      MacOSUtils::computerName();
#elif MZ_ANDROID
      AndroidUtils::getDeviceModel();
#elif MZ_WASM
      "WASM";
#elif MZ_WINDOWS
      QSysInfo::machineHostName() + " " + QSysInfo::productType() + " " +
      WindowsUtils::windowsVersion();
#else
      QSysInfo::machineHostName() + " " + QSysInfo::productType() + " " +
      QSysInfo::productVersion();
#endif

  return deviceName;
}

QString Device::uniqueDeviceId() {
#if MZ_ANDROID
  return AndroidUtils::DeviceId();
#endif
  return QSysInfo::machineUniqueId();
}

Device::Device() { MZ_COUNT_CTOR(Device); }

Device::Device(const Device& other) {
  MZ_COUNT_CTOR(Device);
  *this = other;
}

Device& Device::operator=(const Device& other) {
  if (this == &other) return *this;

  m_deviceName = other.m_deviceName;
  m_uniqueId = other.m_uniqueId;
  m_createdAt = other.m_createdAt;
  m_publicKey = other.m_publicKey;
  m_ipv4Address = other.m_ipv4Address;
  m_ipv6Address = other.m_ipv6Address;

  return *this;
}

Device::~Device() { MZ_COUNT_DTOR(Device); }

bool Device::fromJson(const QJsonValue& json) {
  if (!json.isObject()) {
    return false;
  }

  QJsonObject obj = json.toObject();

  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

  QJsonValue uniqueId = obj.value("unique_id");
  // No checks here.

  QJsonValue pubKey = obj.value("pubkey");
  if (!pubKey.isString()) {
    return false;
  }

  QJsonValue createdAt = obj.value("created_at");
  if (!createdAt.isString()) {
    return false;
  }

  QDateTime date = QDateTime::fromString(createdAt.toString(), Qt::ISODate);
  if (!date.isValid()) {
    return false;
  }

  QJsonValue ipv4Address = obj.value("ipv4_address");
  if (!ipv4Address.isString()) {
    return false;
  }

  QJsonValue ipv6Address = obj.value("ipv6_address");
  if (!ipv6Address.isString()) {
    return false;
  }

  m_deviceName = name.toString();
  m_uniqueId = uniqueId.isString() ? uniqueId.toString() : "";
  m_createdAt = date;
  m_publicKey = pubKey.toString();
  m_ipv4Address = ipv4Address.toString();
  m_ipv6Address = ipv6Address.toString();

  return true;
}

bool Device::isCurrentDevice(const Keys* keys) const {
  Q_ASSERT(keys);
  return m_publicKey == keys->publicKey();
}
