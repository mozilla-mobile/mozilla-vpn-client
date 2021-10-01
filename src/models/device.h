/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICE_H
#define DEVICE_H

#include <QDateTime>
#include <QList>

class Keys;
class QJsonValue;

class Device final {
 public:
  Device();
  Device(const Device& other);
  Device& operator=(const Device& other);
  ~Device();

  static QString currentDeviceName();

  static QString currentDeviceReport();

  static QString uniqueDeviceId();

  [[nodiscard]] bool fromJson(const QJsonValue& json);

  const QString& name() const { return m_deviceName; }

  const QString& uniqueId() const { return m_uniqueId; }

  const QDateTime& createdAt() const { return m_createdAt; }

  bool isDeviceForMigration(const QString& deviceName) const {
    return m_deviceName == deviceName;
  }

  const QString& publicKey() const { return m_publicKey; }

  const QString& ipv4Address() const { return m_ipv4Address; }

  const QString& ipv6Address() const { return m_ipv6Address; }

  bool isCurrentDevice(const Keys* keys) const;

 private:
  QString m_deviceName;
  QString m_uniqueId;
  QDateTime m_createdAt;
  QString m_publicKey;
  QString m_ipv4Address;
  QString m_ipv6Address;
};

#endif  // DEVICE_H
