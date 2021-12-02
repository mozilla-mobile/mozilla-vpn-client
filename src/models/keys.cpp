/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keys.h"
#include "devicemodel.h"
#include "leakdetector.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

Keys::Keys() { MVPN_COUNT_CTOR(Keys); }

Keys::~Keys() { MVPN_COUNT_DTOR(Keys); }

bool Keys::fromSettings() {
  auto& settingsHolder = SettingsHolder::instance();

  if (settingsHolder.privateKey().isEmpty()) {
    return false;
  }

  // Quick migration to retrieve the public key from the current device.
  if (settingsHolder.publicKey().isEmpty()) {
    const QByteArray& json = settingsHolder.devices();
    if (json.isEmpty()) {
      return false;
    }

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

      if (!device.isDeviceForMigration(Device::currentDeviceName())) {
        continue;
      }

      settingsHolder.setPublicKey(device.publicKey());
      break;
    }
  }

  m_privateKey = settingsHolder.privateKey();
  m_publicKey = settingsHolder.publicKey();
  return true;
}

void Keys::storeKeys(const QString& privateKey, const QString& publicKey) {
  m_privateKey = privateKey;
  m_publicKey = publicKey;
}

void Keys::forgetKeys() {
  m_privateKey.clear();
  m_publicKey.clear();
}
