/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractListModel>

#include "device.h"
#include "loghandler.h"

class Keys;

class DeviceModel final : public QAbstractListModel, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceModel)

  Q_PROPERTY(int activeDevices READ activeDevices NOTIFY changed)

 public:
  DeviceModel();
  ~DeviceModel();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
    PublicKeyRole,
    CurrentOneRole,
    CreatedAtRole,
  };

  [[nodiscard]] bool fromJson(const Keys* keys, const QByteArray& s);

  [[nodiscard]] bool fromSettings(const Keys* keys);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  void writeSettings();

  void startDeviceRemovalFromPublicKey(const QString& publicKey);
  void stopDeviceRemovalFromPublicKey(const QString& publicKey,
                                      const Keys* keys);

  void removeDeviceFromPublicKey(const QString& publicKey);

  int activeDevices() const { return static_cast<int>(m_devices.count()); }

  const QList<Device>& devices() const { return m_devices; }

  const Device* currentDevice(const Keys* keys) const;

  bool hasCurrentDevice(const Keys* keys) const;

  const Device* deviceFromPublicKey(const QString& publicKey) const;

  const Device* deviceFromUniqueId() const;

  // LogSerializer interface
  void serializeLogs(
      std::function<void(const QString& name, const QString& logs)>&& callback)
      override;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

 private:
  [[nodiscard]] bool fromJsonInternal(const Keys* keys, const QByteArray& json);

  void moveCurrentDevice(const Keys* keys);
  bool removeRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

 private:
  QByteArray m_rawJson;

  QList<Device> m_devices;

  // This list contains the list of devices that are about to be removed.
  QList<Device> m_removedDevices;
};

#endif  // DEVICEMODEL_H
