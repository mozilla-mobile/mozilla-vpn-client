/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "whatsnewmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "featurelist.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutableListIterator>

namespace {
Logger logger(LOG_MODEL, "WhatsNewModel");
}

WhatsNewModel::WhatsNewModel() { MVPN_COUNT_CTOR(WhatsNewModel); }

WhatsNewModel::~WhatsNewModel() { MVPN_COUNT_DTOR(WhatsNewModel); }

bool WhatsNewModel::fromJson(const Keys* keys, const QByteArray& s) {
  logger.debug() << "WhatsNewModel from json";

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

bool WhatsNewModel::fromSettings(const Keys* keys) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading the device list from settings";

  if (!settingsHolder->hasDevices()) {
    return false;
  }

  const QByteArray& json = settingsHolder->devices();
  if (!fromJsonInternal(keys, json)) {
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

bool WhatsNewModel::fromJsonInternal(const Keys* keys, const QByteArray& json) {
  beginResetModel();

  m_rawJson = "";
  m_devices.clear();

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

  QJsonArray devicesArray = devices.toArray();
  for (QJsonValue deviceValue : devicesArray) {
    Device device;
    if (!device.fromJson(deviceValue)) {
      return false;
    }
    m_devices.append(device);
  }

  std::sort(m_devices.begin(), m_devices.end(),
            std::bind(sortCallback, std::placeholders::_1,
                      std::placeholders::_2, keys));

  endResetModel();
  emit changed();

  return true;
}

void WhatsNewModel::writeSettings() {
  SettingsHolder::instance()->setDevices(m_rawJson);
}

QHash<int, QByteArray> WhatsNewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[PublicKeyRole] = "publicKey";
  roles[CurrentOneRole] = "currentOne";
  roles[CreatedAtRole] = "createdAt";
  return roles;
}

int WhatsNewModel::rowCount(const QModelIndex&) const {
  return m_devices.count();
}

QVariant WhatsNewModel::data(const QModelIndex& index, int role) const {
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

void WhatsNewModel::removeDeviceFromPublicKey(const QString& publicKey) {
  for (int i = 0; i < m_devices.length(); ++i) {
    if (m_devices.at(i).publicKey() == publicKey) {
      removeRow(i);
      emit changed();
      return;
    }
  }
}

bool WhatsNewModel::removeRows(int row, int count, const QModelIndex& parent) {
  Q_ASSERT(count == 1);
  Q_UNUSED(parent);

  beginRemoveRows(parent, row, row);
  m_devices.removeAt(row);
  endRemoveRows();

  return true;
}

const Device* WhatsNewModel::currentDevice(const Keys* keys) const {
  for (const Device& device : m_devices) {
    if (device.isCurrentDevice(keys)) {
      return &device;
    }
  }

  return nullptr;
}

const Device* WhatsNewModel::deviceFromPublicKey(const QString& publicKey) const {
  for (const Device& device : m_devices) {
    if (device.publicKey() == publicKey) {
      return &device;
    }
  }

  return nullptr;
}

bool WhatsNewModel::hasCurrentDevice(const Keys* keys) const {
  return currentDevice(keys) != nullptr;
}

bool WhatsNewModel::doSomething() {
  return false;
}

int WhatsNewModel::featureCount() {
  // Start: Get seen features from settings
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading seen features from settings";
  if (settingsHolder->hasSeenFeatures()) {
    const QStringList& seenFeatureList = settingsHolder->seenFeatures();
    logger.debug() << "WhatsNewModel - seenFeatures: " << seenFeatureList;
  }
  // End: Get seen features from settings


  logger.debug() << "WhatsNewModel - featureCount: " << m_featurelist.size();
  return m_featurelist.size();
}
