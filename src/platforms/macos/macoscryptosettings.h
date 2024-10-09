/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSCRYPTOSETTINGS_H
#define MACOSCRYPTOSETTINGS_H

#include "cryptosettings.h"

class MacOSCryptoSettings final : public CryptoSettings {
 public:
  MacOSCryptoSettings();
  virtual ~MacOSCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(CryptoSettings::Version version,
                    const QByteArray& metadata) override;
  CryptoSettings::Version getPreferredVersion() override {
    return m_keyVersion;
  }

 private:
  static bool checkCodesign();
  static bool checkEntitlement(const QString& name);

  bool m_initialized = false;
  CryptoSettings::Version m_keyVersion = CryptoSettings::NoEncryption;
  QByteArray m_key;
  QString m_appId;
};

#endif  // MACOSCRYPTOSETTINGS_H
