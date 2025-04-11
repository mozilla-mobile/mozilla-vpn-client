/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgcryptosettings.h"

#include <unistd.h>

#include <QScopeGuard>
#include <QSettings>
#include <QtDBus/QtDBus>

#include "constants.h"
#include "cryptosettings.h"
#include "hkdf.h"
#include "logger.h"
#include "settingsholder.h"
#include "xdgportal.h"

namespace {
Logger logger("XdgCryptoSettings");
}  // namespace

XdgCryptoSettings::XdgCryptoSettings()
    : CryptoSettings(), XdgPortal(XDG_PORTAL_SECRET) {
  // Check if we can support cryptosettings.
  auto capabilities = QDBusConnection::sessionBus().connectionCapabilities();
  if ((capabilities & QDBusConnection::UnixFileDescriptorPassing) &&
      (getVersion() >= 1)) {
    m_version = EncryptionChachaPolyV2;

    // Save changes to the "token" to the metadata settings file.
    connect(this, &XdgPortal::xdgResponse, this,
            &XdgCryptoSettings::handleResponse);
  }
}

void XdgCryptoSettings::handleResponse(uint code, QVariantMap results) {
  if (code != 0) {
    return;
  }
  if (results.contains("token")) {
    m_token = results.value("token").toString();
    SettingsHolder::instance()->setXdgSecretToken(m_token);
  }
}

void XdgCryptoSettings::resetKey() {
  m_token.clear();
  m_salt.clear();
  m_key.clear();
}

QDBusMessage XdgCryptoSettings::xdgRetrieveSecret(int fd,
                                                  const QVariantMap& options) {
  // Note: QDBusUnixFileDescriptor() will dup the file descriptor we give it,
  // but we need to be very sure that we close all write ends of the pipe. So
  // the point of this wrapper is to ensure that the QDBusUnixFileDescriptor
  // gets destructed and doesn't leave any open file descriptors.
  return m_portal.call(QDBus::Block, "RetrieveSecret",
                       QVariant::fromValue(QDBusUnixFileDescriptor(fd)),
                       QVariant(options));
}

QByteArray XdgCryptoSettings::xdgReadSecretFile(int fd) {
  QByteArray data;
  for (;;) {
    char buf[PIPE_BUF];
    int len = read(fd, buf, sizeof(buf));
    if (len == 0) {
      // End-of-file.
      break;
    }
    if (len < 0) {
      // An error occured.
      logger.warning() << "Failed to read encryption key:" << strerror(errno);
      return QByteArray();
    }
    // Retrieved more data.
    data.append(buf, len);
  }

  return data;
}

QByteArray XdgCryptoSettings::getKey(CryptoSettings::Version version,
                                     const QByteArray& metadata) {
  if (version != CryptoSettings::EncryptionChachaPolyV2) {
    // This version is not supported.
    return QByteArray();
  }

  // Retrieve the key if we don't already have a copy.
  if (m_key.isEmpty()) {
    QJsonObject obj = QJsonDocument::fromJson(metadata).object();

    // Create a pipe to receive the secret.
    int fds[2];
    if (pipe(fds) != 0) {
      logger.warning() << "Failed to create pipe:" << strerror(errno);
      return QByteArray();
    }
    auto guard = qScopeGuard([&] { close(fds[0]); });

    // Request the secret.
    QVariantMap options;
    options["handle_token"] = QVariant(token());
    if (obj.contains("token")) {
      options["token"] = obj.value("token").toString();
    } else if (!m_token.isEmpty()) {
      options["token"] = m_token;
    }

    QDBusMessage reply = xdgRetrieveSecret(fds[1], options);
    close(fds[1]);  // Close our write end of the pipe right away.
    if (reply.type() != QDBusMessage::ReplyMessage) {
      logger.info() << "Unable to retrieve secret:" << reply.errorMessage();
      return QByteArray();
    } else {
      // We need to rebind our signals if the reply path changed.
      QVariant qv = reply.arguments().value(0);
      setReplyPath(qv.value<QDBusObjectPath>().path());
    }

    // Read the secret.
    m_key = xdgReadSecretFile(fds[0]);
    if (m_key.isEmpty()) {
      // Failed to read the key.
      return QByteArray();
    }

    // Use the salt found in the metadata, or generate a new one.
    if (obj.contains("salt")) {
      m_salt = obj.value("salt").toString();
    } else if (m_salt.isEmpty()) {
      m_salt = generateRandomBytes(32).toBase64();
    }
    // Store the token we used for the request.
    if (options.contains("token")) {
      m_token = options.value("token").toString();
    }
  }

  // Generate the encryption key.
  HKDF hash(QCryptographicHash::Sha256, m_salt.toUtf8());
  hash.addData(m_key);
  return hash.result(CRYPTO_SETTINGS_KEY_SIZE);
}

QByteArray XdgCryptoSettings::getMetaData() {
  QJsonObject obj;
  if (!m_salt.isEmpty()) {
    obj.insert("salt", QJsonValue(m_salt));
  }
  if (!m_token.isEmpty()) {
    obj.insert("token", QJsonValue(m_token));
  }
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}
