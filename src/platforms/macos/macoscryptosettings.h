/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSCRYPTOSETTINGS_H
#define MACOSCRYPTOSETTINGS_H

#include "cryptosettings.h"

class MacOSCryptoSettings final : public CryptoSettings {
 public:
  MacOSCryptoSettings() : CryptoSettings(){};
  virtual ~MacOSCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey() override;
  CryptoSettings::Version getSupportedVersion() override;
};

#endif  // MACOSCRYPTOSETTINGS_H
