/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCRYPTOSETTINGS_H
#define LINUXCRYPTOSETTINGS_H

#include "cryptosettings.h"

class LinuxCryptoSettings final : public CryptoSettings {
 public:
  LinuxCryptoSettings();
  virtual ~LinuxCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(const QByteArray& metadata) override;
  CryptoSettings::Version getSupportedVersion() override;

 private:
  CryptoSettings::Version m_keyVersion = CryptoSettings::NoEncryption;
  bool m_initialized = false;
  QByteArray m_key;
};

#endif  // LINUXCRYPTOSETTINGS_H
