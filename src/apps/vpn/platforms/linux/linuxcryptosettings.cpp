/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QRandomGenerator>

#include "cryptosettings.h"
#include "logger.h"

// No extra QT includes after this line!
#undef Q_SIGNALS
#include <libsecret/secret.h>

const SecretSchema* cryptosettings_get_schema(void) {
  static const SecretSchema cryptosettings_schema = {
      "org.mozilla.vpn.cryptosettings",
      SECRET_SCHEMA_NONE,
      {
          {"NULL", SECRET_SCHEMA_ATTRIBUTE_STRING},
      }};
  return &cryptosettings_schema;
}

namespace {
Logger logger("CryptoSettings");

bool s_initialized = false;
QByteArray s_key;
}  // namespace

void CryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";

  GError* error = nullptr;
  gboolean ok = secret_password_clear_sync(cryptosettings_get_schema(), nullptr,
                                           &error, nullptr);
  if (error != nullptr) {
    logger.error() << "Key reset failed:" << error->message;
    g_error_free(error);
  }

  s_initialized = false;
}

bool CryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (!s_initialized) {
    GError* error = nullptr;
    gchar* password = secret_password_lookup_sync(cryptosettings_get_schema(),
                                                  nullptr, &error, nullptr);
    if (error != nullptr) {
      logger.error() << "Key lookup failed:" << error->message;
      g_error_free(error);
      error = nullptr;
      // fall-through to try creating the password anyways
    }

    if (password != nullptr) {
      QString b64key(password);
      s_key = QByteArray::fromBase64(b64key.toUtf8());
      secret_password_free(password);
    } else {
      logger.debug() << "Key not found. Let's create it.";
      s_key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
      QRandomGenerator* rg = QRandomGenerator::system();
      for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
        s_key[i] = rg->generate() & 0xFF;
      }
      
      QString b64key(s_key.toBase64());
      gboolean ok = secret_password_store_sync(
          cryptosettings_get_schema(), SECRET_COLLECTION_DEFAULT,
          "VPN settings encryption key", qPrintable(b64key), nullptr, &error,
          nullptr);
      if (error != nullptr) {
        logger.error() << "Key storage failed:" << error->message;
        g_error_free(error);
        return false;
      }
    }
  }

  if (s_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(key, s_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }

  logger.warning() << "Invalid key";
  return false;
}

// static
CryptoSettings::Version CryptoSettings::getSupportedVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
