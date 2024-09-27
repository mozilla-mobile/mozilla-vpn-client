/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxcryptosettings.h"

#include <QtDBus/QtDBus>

#include "constants.h"
#include "cryptosettings.h"
#include "logger.h"
#include "xdgcryptosettings.h"

// No extra QT includes after this line!
#undef Q_SIGNALS
#include <libsecret/secret.h>

namespace {
Logger logger("LinuxCryptoSettings");
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

/* The XDG Secrets portal is the preferred way to retrieve secrets from
 * the desktop session, and we should use that whenever possible. However,
 * we keep the LinuxCryptoSettings class around to allow users to migrate
 * from the older libsecrets implementation to the XDG Portal.
 *
 * This class wraps the XdgCryptoSettings implementation, but provides a
 * fallback to fetch a key from libsecret when attempting to decrypt a file
 * using the EncryptionChachaPolyV1 format. For all other cases, we just call
 * through to the XDG implementation.
 */
LinuxCryptoSettings::LinuxCryptoSettings() : XdgCryptoSettings() {
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

  /* Try to lookup the encryption key. This can fail for a variety of reasons.
   * It's also not reliable, which is why we want to move users onto the XDG
   * Secrets portal for managing their encrypted settings.
   */
  GError* err = nullptr;
  auto schema = cryptosettings_get_schema();
  gchar* password = secret_password_lookup_sync(schema, nullptr, &err, nullptr);
  if (err != nullptr) {
    g_error_free(err);
    return;
  } else if (password != nullptr) {
    // We successfully retrieved a key from libsecret.
    logger.info() << "Legacy encryption key found";
    m_legacyKey = QByteArray::fromBase64(QByteArray(password, -1));
    secret_password_free(password);
  }
}

void LinuxCryptoSettings::clearLegacyKey() {
  if (m_legacyKey.isEmpty()) {
    return;
  }
  logger.debug() << "Removing legacy keys from the keychain";
  m_legacyKey.clear();

  GError* error = nullptr;
  auto schema = cryptosettings_get_schema();
  secret_password_clear_sync(schema, nullptr, &error, nullptr);
  if (error != nullptr) {
    logger.error() << "Key reset failed:" << error->message;
    g_error_free(error);
  }
}

void LinuxCryptoSettings::resetKey() {
  clearLegacyKey();
  XdgCryptoSettings::resetKey();
}

QByteArray LinuxCryptoSettings::getKey(Version version,
                                       const QByteArray& metadata) {
  if (version == EncryptionChachaPolyV1) {
    // A legacy key is being requested - get it from the libsecrets API.
    return m_legacyKey;
  } else {
    // Otherwise, use the XDG secrets portal for all new files and keys.
    // Clear the legacy key once an attempt has been made to write a new file.
    clearLegacyKey();
    return XdgCryptoSettings::getKey(version, metadata);
  }
}
