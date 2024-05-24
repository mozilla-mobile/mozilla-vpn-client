/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCRYPTOSETTINGS_H
#define ANDROIDCRYPTOSETTINGS_H

#include <QByteArray>

#include "cryptosettings.h"

class AndroidCryptoSettings final : public CryptoSettings {
 public:
  AndroidCryptoSettings() : CryptoSettings(){};
  virtual ~AndroidCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey() override;
  CryptoSettings::Version getSupportedVersion() override;

 private:
  bool m_initialized = false;
  QByteArray m_key;
};

#endif  // ANDROIDCRYPTOSETTINGS_H
