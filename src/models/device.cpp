/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "device.h"
#include "keys.h"
#include "leakdetector.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

#ifdef QT_DEBUG
#  include <QRandomGenerator>
#endif

#ifdef MVPN_IOS
#  include "platforms/ios/iosutils.h"
#elif MVPN_MACOS
#  include "platforms/macos/macosutils.h"
#elif MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

// static
QString Device::currentDeviceName() {
  QString deviceName =

#ifdef MVPN_IOS
      IOSUtils::computerName();
#elif MVPN_MACOS
      // MacOS has a funny way to rename the hostname based on the network
      // status.
      MacOSUtils::computerName();
#elif MVPN_ANDROID
      AndroidUtils::GetDeviceName();
#elif MVPN_WASM
      "WASM";
#else
      QSysInfo::machineHostName() + " " + QSysInfo::productType() + " " +
      QSysInfo::productVersion();
#endif

  /*  If we want to generate a new device name at each execution, comment out
this block: #ifdef QT_DEBUG static quint32 uniqueId = 0; if (uniqueId == 0) {
      uniqueId = QRandomGenerator::global()->generate();
  }

  deviceName = QString("%1 %2").arg(deviceName).arg(uniqueId);
#endif
  */

  return deviceName;
}

Device::Device() { MVPN_COUNT_CTOR(Device); }

Device::Device(const Device& other) {
  MVPN_COUNT_CTOR(Device);
  *this = other;
}

Device& Device::operator=(const Device& other) {
  if (this == &other) return *this;

  m_deviceName = other.m_deviceName;
  m_createdAt = other.m_createdAt;
  m_publicKey = other.m_publicKey;
  m_ipv4Address = other.m_ipv4Address;
  m_ipv6Address = other.m_ipv6Address;

  return *this;
}

Device::~Device() { MVPN_COUNT_DTOR(Device); }

bool Device::fromJson(const QJsonValue& json) {
  if (!json.isObject()) {
    return false;
  }

  QJsonObject obj = json.toObject();

  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

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
