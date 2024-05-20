/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcryptosettings.h"

#include <QRandomGenerator>
#include <QtDBus/QtDBus>

#include "constants.h"
#include "cryptosettings.h"
#include "logger.h"

// No extra QT includes after this line!
#undef Q_SIGNALS
#include <libsecret/secret.h>

namespace {
Logger logger("CryptoSettings");
}  // namespace

static const SecretSchema* cryptosettings_get_schema(void) {
  static const SecretSchema cryptosettings_schema = {
      Constants::LINUX_CRYPTO_SETTINGS_KEY,
      SECRET_SCHEMA_NONE,
      {
          {"NULL", SECRET_SCHEMA_ATTRIBUTE_STRING},
      }};
  return &cryptosettings_schema;
}

LinuxCryptoSettings::LinuxCryptoSettings() {
  m_keyVersion = CryptoSettings::NoEncryption;

  // Check if "org.freedesktop.secrets" has been taken on the session D-Bus.
  QDBusConnection bus = QDBusConnection::sessionBus();
  QDBusMessage hasOwnerCall = QDBusMessage::createMethodCall(
      "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
      "NameHasOwner");
  hasOwnerCall << QVariant("org.freedesktop.secrets");

  QDBusMessage hasOwner = bus.call(hasOwnerCall, QDBus::Block, 1000);
  if (hasOwner.type() != QDBusMessage::ReplyMessage ||
      hasOwner.arguments().isEmpty() ||
      !hasOwner.arguments().first().toBool()) {
    logger.info() << "Encrypted settings with libsecrets is not supported";
    return;
  }

  // Check if "org.freedesktop.Accounts" "AutomaticLoginUsers" is empty.
  QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                       "org.freedesktop.DBus.Properties",
                       QDBusConnection::systemBus());

  QDBusMessage autoLoginReply =
      iface.call("Get", "org.freedesktop.Accounts", "AutomaticLoginUsers");
  if (autoLoginReply.type() != QDBusMessage::ReplyMessage) {
    logger.info() << "Encrypted settings with libsecrets is not supported "
                     "(incompatible reply)";
    return;
  }

  QVariant v = autoLoginReply.arguments().first();
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
    return;
  }

  // Otherwise - if we passed all the checks then we can use crypto settings.
  m_keyVersion = CryptoSettings::EncryptionChachaPolyV1;
}

void LinuxCryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";

  GError* error = nullptr;
  gboolean ok = secret_password_clear_sync(cryptosettings_get_schema(), nullptr,
                                           &error, nullptr);
  if (!ok) {
    Q_ASSERT(error);
    logger.error() << "Key reset failed:" << error->message;
    g_error_free(error);

    // Fallback to unencrypted settings.
    m_keyVersion = CryptoSettings::NoEncryption;
  }

  m_key.clear();
}

bool LinuxCryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (m_keyVersion == CryptoSettings::NoEncryption) {
    logger.error() << "libsecrets is not supported";
    return false;
  }

  if (m_key.isEmpty()) {
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
      m_key = QByteArray::fromBase64(b64key.toUtf8());
      secret_password_free(password);
    } else {
      logger.debug() << "Key not found. Let's create it.";
      m_key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
      QRandomGenerator* rg = QRandomGenerator::system();
      for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
        m_key[i] = rg->generate() & 0xFF;
      }

      QString b64key(m_key.toBase64());
      gboolean ok = secret_password_store_sync(
          cryptosettings_get_schema(), SECRET_COLLECTION_DEFAULT,
          Constants::LINUX_CRYPTO_SETTINGS_DESC, qPrintable(b64key), nullptr,
          &error, nullptr);
      if (!ok) {
        Q_ASSERT(error);
        logger.error() << "Key storage failed:" << error->message;
        g_error_free(error);

        // Fallback to unencrypted settings.
        m_keyVersion = CryptoSettings::NoEncryption;
        m_key.clear();
        return false;
      }
    }
  }

  if (m_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(key, m_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }

  logger.warning() << "Invalid key";
  return false;
}

CryptoSettings::Version LinuxCryptoSettings::getSupportedVersion() {
  return m_keyVersion;
}
