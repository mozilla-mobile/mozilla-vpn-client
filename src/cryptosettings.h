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
    EncryptionChachaPolyV2,
  };

  static void create();
  static QSettings::Format format();

  // Callback methods for QSetting::Format
  static bool readFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool writeFile(QIODevice& device, const QSettings::SettingsMap& map);

 protected:
  explicit CryptoSettings();
  virtual ~CryptoSettings();

  static QByteArray generateRandomBytes(qsizetype length);

  // Implementations must provide these methods to retrieve keys.
  virtual void resetKey() = 0;
  virtual QByteArray getKey(const QByteArray& metadata) = 0;
  virtual QByteArray getMetaData() { return QByteArray(); }
  virtual Version getSupportedVersion() = 0;

 private:
  // Plantext file implementation
  static bool readJsonFile(QIODevice& device, QSettings::SettingsMap& map);
  static bool writeJsonFile(QIODevice& device,
                            const QSettings::SettingsMap& map);

  // Encrypted V1 and V2 implementation
  bool readEncryptedChachaPolyFile(Version fileVersion, QIODevice& device,
                                   QSettings::SettingsMap& map);

  bool writeEncryptedChachaPolyFile(QIODevice& device,
                                    const QSettings::SettingsMap& map);

 protected:
  uint64_t m_lastNonce = 0;
};

#endif  // CRYPTOSETTINGS_H
