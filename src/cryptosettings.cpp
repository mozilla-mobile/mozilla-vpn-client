/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cryptosettings.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

#include "hacl-star/Hacl_Chacha20Poly1305_32.h"
#include "logger.h"

constexpr int NONCE_SIZE = 12;
constexpr int MAC_SIZE = 16;

namespace {

Logger logger("CryptoSettings");

uint64_t lastNonce = 0;

}  // namespace

// static
bool CryptoSettings::readFile(QIODevice& device, QSettings::SettingsMap& map) {
  QByteArray version = device.read(1);
  if (version.length() != 1) {
    logger.error() << "Failed to read the version";
    return false;
  }

  switch ((CryptoSettings::Version)version.at(0)) {
    case NoEncryption:
      return readJsonFile(device, map);
    case EncryptionChachaPolyV1:
      return readEncryptedChachaPolyV1File(device, map);
    default:
      logger.error() << "Unsupported version";
      return false;
  }
}

// static
bool CryptoSettings::readJsonFile(QIODevice& device,
                                  QSettings::SettingsMap& map) {
  QByteArray content = device.readAll();

  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.error() << "Invalid content read from the JSON file";
    return false;
  }

  QJsonObject obj = json.object();
  for (QJsonObject::const_iterator i = obj.constBegin(); i != obj.constEnd();
       ++i) {
    map.insert(i.key(), i.value().toVariant());
  }

  return true;
}

// static
bool CryptoSettings::readEncryptedChachaPolyV1File(
    QIODevice& device, QSettings::SettingsMap& map) {
  QByteArray nonce = device.read(NONCE_SIZE);
  if (nonce.length() != NONCE_SIZE) {
    logger.error() << "Failed to read the nonce";
    return false;
  }

  QByteArray mac = device.read(MAC_SIZE);
  if (mac.length() != MAC_SIZE) {
    logger.error() << "Failed to read the MAC";
    return false;
  }

  QByteArray ciphertext = device.readAll();
  if (ciphertext.length() == 0) {
    logger.error() << "Failed to read the ciphertext";
    return false;
  }

  uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
  if (!getKey(key)) {
    logger.error() << "Something went wrong reading the key";
    return false;
  }

  QByteArray version(1, EncryptionChachaPolyV1);
  QByteArray content(ciphertext.length(), 0x00);
  uint32_t result = Hacl_Chacha20Poly1305_32_aead_decrypt(
      key, (uint8_t*)nonce.data(), static_cast<uint32_t>(version.length()),
      (uint8_t*)version.data(), static_cast<uint32_t>(ciphertext.length()),
      (uint8_t*)content.data(), (uint8_t*)ciphertext.data(),
      (uint8_t*)mac.data());
  if (result != 0) {
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.error() << "Invalid content read from the JSON file";
    return false;
  }

  QJsonObject obj = json.object();
  for (QJsonObject::const_iterator i = obj.constBegin(); i != obj.constEnd();
       ++i) {
    map.insert(i.key(), i.value().toVariant());
  }

  Q_ASSERT(NONCE_SIZE > sizeof(lastNonce));
  memcpy(&lastNonce, nonce.data(), sizeof(lastNonce));

  return true;
}

// static
bool CryptoSettings::writeFile(QIODevice& device,
                               const QSettings::SettingsMap& map) {
  logger.debug() << "Writing the settings file";

  Version version = getSupportedVersion();
  if (!writeVersion(device, version)) {
    logger.error() << "Failed to write the version";
    return false;
  }

  switch (version) {
    case NoEncryption:
      return writeJsonFile(device, map);
    case EncryptionChachaPolyV1:
      return writeEncryptedChachaPolyV1File(device, map);
    default:
      logger.error() << "Unsupported version.";
      return false;
  }
}

// static
bool CryptoSettings::writeVersion(QIODevice& device,
                                  CryptoSettings::Version version) {
  QByteArray v(1, version);
  return device.write(v) == v.length();
}

// static
bool CryptoSettings::writeJsonFile(QIODevice& device,
                                   const QSettings::SettingsMap& map) {
  logger.debug() << "Write plaintext JSON file";

  QJsonObject obj;
  for (QSettings::SettingsMap::ConstIterator i = map.begin(); i != map.end();
       ++i) {
    obj.insert(i.key(), QJsonValue::fromVariant(i.value()));
  }

  QJsonDocument json;
  json.setObject(obj);
  QByteArray content = json.toJson(QJsonDocument::Compact);

  if (device.write(content) != content.length()) {
    logger.error() << "Failed to write the content";
    return false;
  }

  return true;
}

// static
bool CryptoSettings::writeEncryptedChachaPolyV1File(
    QIODevice& device, const QSettings::SettingsMap& map) {
  logger.debug() << "Write encrypted file";

  QJsonObject obj;
  for (QSettings::SettingsMap::ConstIterator i = map.begin(); i != map.end();
       ++i) {
    obj.insert(i.key(), QJsonValue::fromVariant(i.value()));
  }

  QJsonDocument json;
  json.setObject(obj);
  QByteArray content = json.toJson(QJsonDocument::Compact);

  logger.debug() << "Incrementing nonce:" << lastNonce;
  if (++lastNonce == UINT64_MAX) {
    logger.debug() << "Reset the nonce and the key.";
    resetKey();
    lastNonce = 0;
  }

  Q_ASSERT(NONCE_SIZE > sizeof(lastNonce));
  QByteArray nonce(NONCE_SIZE, 0x00);
  memcpy(nonce.data(), &lastNonce, sizeof(lastNonce));

  uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
  if (!getKey(key)) {
    logger.debug() << "Invalid key";
    return false;
  }

  QByteArray version(1, EncryptionChachaPolyV1);
  QByteArray ciphertext(content.length(), 0x00);
  QByteArray mac(MAC_SIZE, 0x00);

  Hacl_Chacha20Poly1305_32_aead_encrypt(
      key, (uint8_t*)nonce.data(), static_cast<uint32_t>(version.length()),
      (uint8_t*)version.data(), static_cast<uint32_t>(content.length()),
      (uint8_t*)content.data(), (uint8_t*)ciphertext.data(),
      (uint8_t*)mac.data());

  if (device.write(nonce) != nonce.length()) {
    logger.error() << "Failed to write the nonce";
    return false;
  }

  if (device.write(mac) != mac.length()) {
    logger.error() << "Failed to write the MAC";
    return false;
  }

  if (device.write(ciphertext) != ciphertext.length()) {
    logger.error() << "Failed to write the cipher";
    return false;
  }

  return true;
}
