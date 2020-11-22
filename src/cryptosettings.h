/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRYPTOSETTINGS_H
#define CRYPTOSETTINGS_H

#include <QSettings>

constexpr int CRYPTO_SETTINGS_KEY_SIZE = 32;

class CryptoSettings final {
 public:
  enum Version {
    NoEncryption,
    EncryptionChachaPolyV1,
  };

  static bool readFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool writeFile(QIODevice& device, const QSettings::SettingsMap& map);

 private:
  static void resetKey();
  static bool getKey(uint8_t[CRYPTO_SETTINGS_KEY_SIZE]);

  static Version getSupportedVersion();
  static bool writeVersion(QIODevice& device, Version version);

  static bool readJsonFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool readEncryptedChachaPolyV1File(QIODevice& device,
                                            QSettings::SettingsMap& map);

  static bool writeJsonFile(QIODevice& device,
                            const QSettings::SettingsMap& map);
  static bool writeEncryptedChachaPolyV1File(QIODevice& device,
                                             const QSettings::SettingsMap& map);
};

#endif  // CRYPTOSETTINGS_H
