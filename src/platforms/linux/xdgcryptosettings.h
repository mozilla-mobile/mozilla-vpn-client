/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGCRYPTOSETTINGS_H
#define XDGCRYPTOSETTINGS_H

#include "cryptosettings.h"
#include "xdgportal.h"

#include <QDBusMessage>

class XdgCryptoSettings final : public CryptoSettings, public XdgPortal {
 public:
  XdgCryptoSettings();
  virtual ~XdgCryptoSettings() = default;

  void resetKey() override;
  bool getKey(uint8_t output[CRYPTO_SETTINGS_KEY_SIZE]) override;
  CryptoSettings::Version getSupportedVersion() override;

 private:
   QDBusMessage xdgRetrieveSecret(int fd, const QVariantMap& options);
};

#endif  // XDGCRYPTOSETTINGS_H
