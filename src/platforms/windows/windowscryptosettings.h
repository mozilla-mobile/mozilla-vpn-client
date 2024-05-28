/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCRYPTOSETTINGS_H
#define WINDOWSCRYPTOSETTINGS_H

#include "cryptosettings.h"

class WindowsCryptoSettings final : public CryptoSettings {
 public:
  WindowsCryptoSettings() : CryptoSettings(){};
  virtual ~WindowsCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(const QByteArray& metadata) override;
  CryptoSettings::Version getSupportedVersion() override;

 private:
  bool m_initialized = false;
  QByteArray m_key;
};

#endif  // WINDOWSCRYPTOSETTINGS_H
