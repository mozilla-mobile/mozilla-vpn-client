/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <jni.h>

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QRandomGenerator>
#include <QTimer>

#include "androidcommons.h"
#include "cryptosettings.h"
#include "logging/logger.h"

constexpr auto KEYSTORE_CLASS =
    "org/mozilla/firefox/qt/common/SettingsKeystore";

namespace {
Logger logger("AndroidKeystore");

inline bool jni_hasKey() {
  return (bool)QJniObject::callStaticMethod<jboolean>(KEYSTORE_CLASS, "hasKey",
                                                      "()Z");
}

inline void jni_setString(QString value) {
  QJniObject::callStaticMethod<void>(KEYSTORE_CLASS, "setKey",
                                     "(Ljava/lang/String;)V",
                                     QJniObject::fromString(value).object());
}

inline void jni_setKey(QByteArray value) {
  return jni_setString(QString(value.toBase64()));
}

inline QByteArray jni_getKey() {
  QJniEnvironment env;
  QJniObject string = QJniObject::callStaticObjectMethod(
      KEYSTORE_CLASS, "getKey", "()Ljava/lang/String;");
  jstring value = (jstring)string.object();
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    return QByteArray();
  }
  QString b64key(buffer);
  return QByteArray::fromBase64(b64key.toUtf8());
}

inline void jni_clear() {
  QJniObject::callStaticMethod<void>(KEYSTORE_CLASS, "clear", "()V");
}

bool initialized = false;
QByteArray key;
}  // namespace

// static
void CryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";
  jni_clear();
  initialized = false;
}

// static
bool CryptoSettings::getKey(uint8_t output[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (initialized) {
    if (key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
      memcpy(output, key.data(), CRYPTO_SETTINGS_KEY_SIZE);
      return true;
    }
    logger.warning() << "Malformed key?";
  }

  if (!jni_hasKey()) {
    logger.warning() << "Key not found. Let's create it.";
    key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
    QRandomGenerator* rg = QRandomGenerator::system();
    for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
      key[i] = rg->generate() & 0xFF;
    }
    jni_setKey(key);
    if (key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
      memcpy(output, key.data(), CRYPTO_SETTINGS_KEY_SIZE);
      return true;
    }
  }

  key = jni_getKey();
  if (key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(output, key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }
  return false;
}

// static
CryptoSettings::Version CryptoSettings::getSupportedVersion() {
  logger.debug() << "Get supported settings method";

  uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
  if (getKey(key)) {
    logger.debug() << "Encryption supported!";
    return CryptoSettings::EncryptionChachaPolyV1;
  }
  logger.debug() << "No encryption";
  return CryptoSettings::NoEncryption;
}
