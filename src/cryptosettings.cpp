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

#if defined(UNIT_TEST)
#  include "platforms/dummy/dummycryptosettings.h"
#elif defined(MZ_FLATPAK)
#  include "platforms/linux/xdgcryptosettings.h"
#elif defined(MZ_LINUX)
#  include "platforms/linux/linuxcryptosettings.h"
#elif defined(MZ_MACOS) || defined(MZ_IOS)
#  include "platforms/macos/macoscryptosettings.h"
#elif defined(MZ_WINDOWS)
#  include "platforms/windows/windowscryptosettings.h"
#elif defined(MZ_ANDROID)
#  include "platforms/android/androidcryptosettings.h"
#endif

constexpr int NONCE_SIZE = 12;
constexpr int MAC_SIZE = 16;
constexpr int ENCRYPTED_V2_HEADER_SIZE = 4;

namespace {
Logger logger("CryptoSettings");
CryptoSettings* s_instance = nullptr;
}  // namespace

// static
QSettings::Format CryptoSettings::format() {
  static QSettings::Format format =
      QSettings::registerFormat("moz", readFile, writeFile);

  // Create a platform crypto settings implementation, if supported.
  if (!s_instance) {
#if defined(UNIT_TEST)
    s_instance = new DummyCryptoSettings();
#elif defined(MZ_FLATPAK)
    s_instance = new XdgCryptoSettings();
#elif defined(MZ_LINUX)
    s_instance = new LinuxCryptoSettings();
#elif defined(MZ_MACOS) || defined(MZ_IOS)
    s_instance = new MacOSCryptoSettings();
#elif defined(MZ_WINDOWS)
    s_instance = new WindowsCryptoSettings();
#elif defined(MZ_ANDROID)
    s_instance = new AndroidCryptoSettings();
#endif
  }

  return format;
}

CryptoSettings::CryptoSettings() {
  logger.info() << "Creating CryptoSettings";
  if (s_instance) {
    delete s_instance;
  }
  s_instance = this;
}

CryptoSettings::~CryptoSettings() {
  logger.info() << "Destroying CryptoSettings";
  s_instance = nullptr;
}

// static
bool CryptoSettings::readFile(QIODevice& device, QSettings::SettingsMap& map) {
  QByteArray version = device.read(1);
  if (version.length() != 1) {
    logger.error() << "Failed to read the version";
    return false;
  }

  // Plaintext settings are always supported.
  CryptoSettings::Version fileVersion = (CryptoSettings::Version)version.at(0);
  if (fileVersion == NoEncryption) {
    return readJsonFile(device, map);
  }

  // All other crypto settings require an implementation.
  if (!s_instance) {
    logger.error() << "No supported CryptoSettings implementation found";
    return false;
  }

  switch (fileVersion) {
    case NoEncryption:
      Q_UNREACHABLE();
      return false;

    case EncryptionChachaPolyV1:
      // Fall-through.
    case EncryptionChachaPolyV2:
      return s_instance->readEncryptedChachaPolyFile(fileVersion, device, map);

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

bool CryptoSettings::readEncryptedChachaPolyFile(
    Version fileVersion, QIODevice& device, QSettings::SettingsMap& map) {
  QByteArray header(1, fileVersion);
  QByteArray metadata;
  if (fileVersion == EncryptionChachaPolyV2) {
    // Encrypted V2 Header:
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | Version | Reserved  | Metadata Length | Metadata  |
    // |  8-bit  |   8-bit   |     16-bit      | variable  |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    header.append(device.read(ENCRYPTED_V2_HEADER_SIZE - 1));
    if (header.length() != ENCRYPTED_V2_HEADER_SIZE) {
      logger.error() << "Failed to read encrypted v2 header";
      return false;
    }
    quint16 metaLength = header[2] + (header[3] >> 8);
    header.append(device.read(metaLength));
    if (header.length() != (metaLength + ENCRYPTED_V2_HEADER_SIZE)) {
      logger.error() << "Failed to read metadata";
      return false;
    }
    metadata = header.mid(ENCRYPTED_V2_HEADER_SIZE);
  } else if (fileVersion != EncryptionChachaPolyV1) {
    logger.error() << "Unsupported encrypted file version:" << header[0];
    return false;
  }

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

  QByteArray key = getKey(metadata);
  if (key.isEmpty()) {
    logger.error() << "Something went wrong reading the key";
    return false;
  }
  if (key.length() != CRYPTO_SETTINGS_KEY_SIZE) {
    logger.error() << "Invalid key length:" << key.length();
    return false;
  }

  QByteArray content(ciphertext.length(), 0x00);
  uint32_t result = Hacl_Chacha20Poly1305_32_aead_decrypt(
      (uint8_t*)key.data(), (uint8_t*)nonce.data(),
      static_cast<uint32_t>(header.length()), (uint8_t*)header.data(),
      static_cast<uint32_t>(ciphertext.length()), (uint8_t*)content.data(),
      (uint8_t*)ciphertext.data(), (uint8_t*)mac.data());
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

  Q_ASSERT(NONCE_SIZE > sizeof(m_lastNonce));
  memcpy(&m_lastNonce, nonce.data(), sizeof(m_lastNonce));

  return true;
}

// static
bool CryptoSettings::writeFile(QIODevice& device,
                               const QSettings::SettingsMap& map) {
  logger.debug() << "Writing the settings file";
  CryptoSettings::Version version = NoEncryption;

  if (s_instance) {
    version = s_instance->getSupportedVersion();
  }

  switch (version) {
    case NoEncryption:
      return writeJsonFile(device, map);
    case EncryptionChachaPolyV1:
      // Fall-through
    case EncryptionChachaPolyV2:
      Q_ASSERT(s_instance);
      return s_instance->writeEncryptedChachaPolyFile(device, map);
    default:
      logger.error() << "Unsupported version.";
      return false;
  }
}

// static
bool CryptoSettings::writeJsonFile(QIODevice& device,
                                   const QSettings::SettingsMap& map) {
  if (!device.putChar(CryptoSettings::NoEncryption)) {
    logger.error() << "Failed to write the version";
    return false;
  }

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

bool CryptoSettings::writeEncryptedChachaPolyFile(
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

  logger.debug() << "Incrementing nonce:" << m_lastNonce;
  if (++m_lastNonce == UINT64_MAX) {
    logger.debug() << "Reset the nonce and the key.";
    resetKey();
    m_lastNonce = 0;
  }

  Q_ASSERT(NONCE_SIZE > sizeof(m_lastNonce));
  QByteArray nonce(NONCE_SIZE, 0x00);
  memcpy(nonce.data(), &m_lastNonce, sizeof(m_lastNonce));

  QByteArray key = getKey(QByteArray());
  if (key.isEmpty()) {
    logger.error() << "Something went wrong reading the key";
    return false;
  }
  if (key.length() != CRYPTO_SETTINGS_KEY_SIZE) {
    logger.error() << "Invalid key length:" << key.length();
    return false;
  }

  QByteArray header;
  QByteArray metadata = getMetaData();
  if (metadata.isEmpty()) {
    // Encrypted V1 Header: Just the version.
    header.append(1, CryptoSettings::EncryptionChachaPolyV1);
  } else if (metadata.length() <= UINT16_MAX) {
    // Encrypted V2 Header:
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | Version | Reserved  | Metadata Length | Metadata  |
    // |  8-bit  |   8-bit   |     16-bit      | variable  |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    header.reserve(ENCRYPTED_V2_HEADER_SIZE + metadata.length());
    header.append(1, CryptoSettings::EncryptionChachaPolyV2);
    header.append(1, 0);
    header.append(1, metadata.length() & 0xff);
    header.append(1, (metadata.length() >> 8) & 0xff);
    header.append(metadata);
  } else {
    logger.error() << "Failed to write encrypted file header: too long";
    return false;
  }

  QByteArray ciphertext(content.length(), 0x00);
  QByteArray mac(MAC_SIZE, 0x00);

  Hacl_Chacha20Poly1305_32_aead_encrypt(
      (uint8_t*)key.data(), (uint8_t*)nonce.data(),
      static_cast<uint32_t>(header.length()), (uint8_t*)header.data(),
      static_cast<uint32_t>(content.length()), (uint8_t*)content.data(),
      (uint8_t*)ciphertext.data(), (uint8_t*)mac.data());

  if (device.write(header) != header.length()) {
    logger.error() << "Failed to write the header";
    return false;
  }

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

// static
QByteArray CryptoSettings::generateRandomBytes(qsizetype length) {
  QRandomGenerator* rg = QRandomGenerator::system();

  QByteArray value;
  value.reserve(length + sizeof(quint32) - 1);

  for (qsizetype i = 0; i < length; i += sizeof(quint32)) {
    quint32 x = rg->generate();
    value.append((const char*)&x, sizeof(x));
  }
  return value.left(length);
}
