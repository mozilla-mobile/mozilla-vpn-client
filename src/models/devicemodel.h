/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractListModel>
#include <QPointer>

#include "device.h"

class DeviceModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceModel)

  Q_PROPERTY(int activeDevices READ activeDevices NOTIFY changed)

 public:
  DeviceModel();
  ~DeviceModel();

  enum ServerCountryRoles {
    NameRole = Qt::UserRole + 1,
    CurrentOneRole,
    CreatedAtRole,
  };

  [[nodiscard]] bool fromJson(const QByteArray& s);

  [[nodiscard]] bool fromSettings();

  bool initialized() const { return !m_rawJson.isEmpty(); }

  void writeSettings();

  bool hasDevice(const QString& deviceName) const;

  void removeDevice(const QString& deviceName);

  const Device* device(const QString& deviceName) const;

  int activeDevices() const { return m_devices.count(); }

  const QList<Device>& devices() const { return m_devices; }

  const Device* currentDevice() const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

 private:
  [[nodiscard]] bool fromJsonInternal(const QByteArray& json);

  bool removeRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

 private:
  QByteArray m_rawJson;

  QList<Device> m_devices;
};

#endif  // DEVICEMODEL_H
