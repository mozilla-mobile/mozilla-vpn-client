/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRYPTOSETTINGS_H
#define CRYPTOSETTINGS_H

#include <QByteArray>
#include <QSettings>

constexpr int CRYPTO_SETTINGS_KEY_SIZE = 32;

class CryptoSettings {
 public:
  enum Version {
    NoEncryption,
    EncryptionChachaPolyV1,
  };

  explicit CryptoSettings();
  virtual ~CryptoSettings();

  static QSettings::Format format();
  static QByteArray generateRandomBytes(qsizetype length);

  // Callback methods for QSetting::Format
  static bool readFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool writeFile(QIODevice& device, const QSettings::SettingsMap& map);

 private:
  // Implementations must provide these methods to retrieve keys.
  virtual void resetKey() = 0;
  virtual QByteArray getKey() = 0;
  virtual Version getSupportedVersion() = 0;

  static bool writeVersion(QIODevice& device, Version version);
  static bool readJsonFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool writeJsonFile(QIODevice& device,
                            const QSettings::SettingsMap& map);

  bool readEncryptedChachaPolyV1File(QIODevice& device,
                                     QSettings::SettingsMap& map);

  bool writeEncryptedChachaPolyV1File(QIODevice& device,
                                      const QSettings::SettingsMap& map);

 protected:
  static void addProvider(CryptoSettings* provider);

 protected:
  uint64_t m_lastNonce = 0;
};

#endif  // CRYPTOSETTINGS_H
