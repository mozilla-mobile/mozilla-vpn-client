/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keys.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "settingsholder.h"

Keys::Keys() { MVPN_COUNT_CTOR(Keys); }

Keys::~Keys() { MVPN_COUNT_DTOR(Keys); }

bool Keys::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (!settingsHolder->hasPrivateKey()) {
    return false;
  }

  // Quick migration to retrieve the public key from the current device.
  if (!settingsHolder->hasPublicKey()) {
    MozillaVPN* vpn = MozillaVPN::instance();
    const Device* device = vpn->deviceModel()->currentDevice();
    if (!device) {
      return false;
    }

    settingsHolder->setPublicKey(device->publicKey());
  }

  m_privateKey = settingsHolder->privateKey();
  m_publicKey = settingsHolder->publicKey();
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
