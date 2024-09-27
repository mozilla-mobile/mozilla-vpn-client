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
  m_xdg = new XdgCryptoSettings();

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

  // Try to lookup the encryption key this can fail for a variety of reasons.
  // It's also not reliable, which is why we want to move users onto the XDG
  // Secrets portal for managing their encrypted settings.
  GError* error = nullptr;
  gchar* password = secret_password_lookup_sync(cryptosettings_get_schema(),
                                                nullptr, &error, nullptr);
  if (error != nullptr) {
    logger.error() << "Key lookup failed:" << error->message;
    g_error_free(error);
    error = nullptr;
    // fall-through to try creating the password anyways
  } if (password != nullptr) {
    // We successfully retrieved a key from libsecret.
    QString b64key(password);
    m_legacyKey = QByteArray::fromBase64(b64key.toUtf8());
    secret_password_free(password);
  }
}

void LinuxCryptoSettings::resetKey() {
  // Reset the key in the XDG secrets portal.
  m_xdg->resetKey();

  logger.debug() << "Reset the key in the keychain";

  GError* error = nullptr;
  auto schema = cryptosettings_get_schema();
  gboolean ok = secret_password_clear_sync(schema, nullptr, &error, nullptr);
  if (!ok) {
    Q_ASSERT(error);
    logger.error() << "Key reset failed:" << error->message;
    g_error_free(error);
  }

  m_legacyKey.clear();
}

QByteArray LinuxCryptoSettings::getKey(Version version, const QByteArray& metadata) {
  if (version == EncryptionChachaPolyV1) {
    // A legacy key is being requested - get it from the libsecrets API.
    return m_legacyKey;
  } else {
    // Otherwise, use the XDG secrets portal for all new files and keys.
    return m_xdg->getKey(version, metadata);
  }
}

CryptoSettings::Version LinuxCryptoSettings::getPreferredVersion() {
  return m_xdg->getPreferredVersion();
}

QByteArray LinuxCryptoSettings::getMetaData() {
  return m_xdg->getMetaData();
}
