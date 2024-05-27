/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYCRYPTOSETTINGS_H
#define DUMMYCRYPTOSETTINGS_H

#include "cryptosettings.h"

// A mocked implementation that returns a fixed key.
class DummyCryptoSettings final : public CryptoSettings {
 public:
  DummyCryptoSettings() : CryptoSettings(){};
  virtual ~DummyCryptoSettings() = default;

  virtual void resetKey() override{ m_fakeKeyValue++; };
  virtual QByteArray getKey() override {
    return QByteArray(CRYPTO_SETTINGS_KEY_SIZE, m_fakeKeyValue);
  };
  virtual Version getSupportedVersion() override { return m_keyVersion; };

 private:
  char m_fakeKeyValue = 'A';
  Version m_keyVersion = CryptoSettings::EncryptionChachaPolyV1;

  friend class TestCryptoSettings;
};

#endif  // DUMMYCRYPTOSETTINGS_H
