/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WHATSNEWMODEL_H
#define WHATSNEWMODEL_H

#include <QAbstractListModel>
#include <QPointer>

#include "device.h"
#include "featurelist.h"

class Keys;
class Feature;

class WhatsNewModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WhatsNewModel)

  Q_PROPERTY(int activeDevices READ activeDevices NOTIFY changed)

  Q_PROPERTY(int newFeatures READ newFeatures NOTIFY newFeaturesChanged)

  Q_PROPERTY(int hasUnseenFeature READ hasUnseenFeature NOTIFY
             hasUnseenFeatureChanged)

 public:
  WhatsNewModel();
  ~WhatsNewModel();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
    PublicKeyRole,
    CurrentOneRole,
    CreatedAtRole,
  };

  [[nodiscard]] bool fromJson(const Keys* keys, const QByteArray& s);

  [[nodiscard]] bool fromSettings(const Keys* keys);

  void initialize();

  bool initialized() const { return !m_rawJson.isEmpty(); }

  void writeSettings();

  void removeDeviceFromPublicKey(const QString& publicKey);

  int activeDevices() const { return m_devices.count(); }

  int hasUnseenFeature() const { return m_devices.count(); }

  int newFeatures() const {
    return m_featurelist.count();
  }

  Q_INVOKABLE bool doSomething();

  Q_INVOKABLE int featureCount();

  const QList<Device>& devices() const { return m_devices; }

  const Device* currentDevice(const Keys* keys) const;

  bool hasCurrentDevice(const Keys* keys) const;

  const Device* deviceFromPublicKey(const QString& publicKey) const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

  void hasUnseenFeatureChanged();

  void newFeaturesChanged();

 private:
  [[nodiscard]] bool fromJsonInternal(const Keys* keys, const QByteArray& json);

  bool removeRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

 private:
  QByteArray m_rawJson;

  QList<Device> m_devices;

  QList<Feature*> m_featurelist;
};

#endif  // WHATSNEWMODEL_H
