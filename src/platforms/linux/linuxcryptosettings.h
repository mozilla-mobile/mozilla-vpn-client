/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCRYPTOSETTINGS_H
#define LINUXCRYPTOSETTINGS_H

#include "cryptosettings.h"

class XdgCryptoSettings;

class LinuxCryptoSettings final : public CryptoSettings {
 public:
  LinuxCryptoSettings();
  virtual ~LinuxCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(CryptoSettings::Version version,
                    const QByteArray& metadata) override;
  QByteArray getMetaData() override;
  CryptoSettings::Version getPreferredVersion() override;

 private:
  // Holds a legeacy encryption key, if we could find one.
  QByteArray m_legacyKey;

  // The real cryptosettings implementation.
  XdgCryptoSettings* m_xdg = nullptr;
};

#endif  // LINUXCRYPTOSETTINGS_H
