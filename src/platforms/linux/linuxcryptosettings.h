/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCRYPTOSETTINGS_H
#define LINUXCRYPTOSETTINGS_H

#include "cryptosettings.h"
#include "xdgcryptosettings.h"

class LinuxCryptoSettings final : public XdgCryptoSettings {
 public:
  LinuxCryptoSettings();
  virtual ~LinuxCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(CryptoSettings::Version version,
                    const QByteArray& metadata) override;

 private:
  void clearLegacyKey();

  // Holds a legeacy encryption key, if we could find one.
  QByteArray m_legacyKey;
};

#endif  // LINUXCRYPTOSETTINGS_H
