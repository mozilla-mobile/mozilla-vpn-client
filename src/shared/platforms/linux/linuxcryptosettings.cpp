/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QRandomGenerator>
#include <QtDBus/QtDBus>

#include "appconstants.h"
#include "cryptosettings.h"
#include "logger.h"

// No extra QT includes after this line!
#undef Q_SIGNALS
#include <libsecret/secret.h>

const SecretSchema* cryptosettings_get_schema(void) {
  static const SecretSchema cryptosettings_schema = {
      AppConstants::LINUX_CRYPTO_SETTINGS_KEY,
      SECRET_SCHEMA_NONE,
      {
          {"NULL", SECRET_SCHEMA_ATTRIBUTE_STRING},
      }};
  return &cryptosettings_schema;
}

namespace {
Logger logger("CryptoSettings");

CryptoSettings::Version s_keyVersion = CryptoSettings::NoEncryption;
bool s_initialized = false;
QByteArray s_key;
}  // namespace

void CryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";

  GError* error = nullptr;
  gboolean ok = secret_password_clear_sync(cryptosettings_get_schema(), nullptr,
                                           &error, nullptr);
  if (!ok) {
    Q_ASSERT(error);
    logger.error() << "Key reset failed:" << error->message;
    g_error_free(error);

    // Fallback to unencrypted settings.
    s_keyVersion = CryptoSettings::NoEncryption;
  }

  s_key.clear();
}

bool CryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (!s_initialized) {
    s_keyVersion = getSupportedVersion();
  }
  if (s_keyVersion == CryptoSettings::NoEncryption) {
    logger.error() << "libsecrets is not supported";
    return false;
  }

  if (s_key.isEmpty()) {
    // Try to lookup the encryption key.
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
          AppConstants::LINUX_CRYPTO_SETTINGS_DESC, qPrintable(b64key), nullptr,
          &error, nullptr);
      if (!ok) {
        Q_ASSERT(error);
        logger.error() << "Key storage failed:" << error->message;
        g_error_free(error);

        // Fallback to unencrypted settings.
        s_keyVersion = CryptoSettings::NoEncryption;
        s_key.clear();
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
  if (!s_initialized) {
    s_initialized = true;

    // Check if "org.freedesktop.secrets" has been taken on the session D-Bus.
    {
      QDBusConnection bus = QDBusConnection::sessionBus();
      QDBusMessage hasOwnerCall = QDBusMessage::createMethodCall(
          "org.freedesktop.DBus", "/org/freedesktop/DBus",
          "org.freedesktop.DBus", "NameHasOwner");
      hasOwnerCall << QVariant("org.freedesktop.secrets");

      QDBusMessage reply = bus.call(hasOwnerCall, QDBus::Block, 1000);
      if (reply.type() != QDBusMessage::ReplyMessage ||
          reply.arguments().isEmpty() || !reply.arguments().first().toBool()) {
        logger.info() << "Encrypted settings with libsecrets is not supported";
        return s_keyVersion;
      }
    }

    // Check if "org.freedesktop.Accounts" "AutomaticLoginUsers" is empty.
    {
      QDBusInterface iface(
          "org.freedesktop.Accounts", "/org/freedesktop/Accounts",
          "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

      QDBusMessage reply =
          iface.call("Get", "org.freedesktop.Accounts", "AutomaticLoginUsers");

      if (reply.type() != QDBusMessage::ReplyMessage) {
        logger.info() << "Encrypted settings with libsecrets is not supported "
                         "(incompatible reply)";
        return s_keyVersion;
      }

      QVariant v = reply.arguments().first();
      QDBusArgument arg =
          v.value<QDBusVariant>().variant().value<QDBusArgument>();

      QStringList users;
      arg.beginArray();
      while (!arg.atEnd()) {
        QString user;
        arg >> user;
        users << user;
      }
      arg.endArray();

      if (!users.isEmpty()) {
        logger.info() << "Encrypted settings with libsecrets is not supported "
                         "with auto-login";
        return s_keyVersion;
      }
    }

    s_keyVersion = CryptoSettings::EncryptionChachaPolyV1;
  }

  return s_keyVersion;
}
