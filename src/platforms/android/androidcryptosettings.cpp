/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcryptosettings.h"

#include <jni.h>

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QTimer>

#include "androidcommons.h"
#include "cryptosettings.h"
#include "logger.h"

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

}  // namespace

void AndroidCryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";
  jni_clear();
  m_initialized = false;
}

QByteArray AndroidCryptoSettings::getKey() {
  if (m_initialized) {
    return m_key;
    logger.warning() << "Malformed key?";
  }

  if (!jni_hasKey()) {
    logger.warning() << "Key not found. Let's create it.";
    m_key = generateRandomBytes(CRYPTO_SETTINGS_KEY_SIZE);
    jni_setKey(m_key);
    return m_key;
  }

  return jni_getKey();
}

CryptoSettings::Version AndroidCryptoSettings::getSupportedVersion() {
  logger.debug() << "Get supported settings method";

  if (getKey().isEmpty()) {
    logger.debug() << "No encryption";
    return CryptoSettings::NoEncryption;
  }
  logger.debug() << "Encryption supported!";
  return CryptoSettings::EncryptionChachaPolyV1;
}
