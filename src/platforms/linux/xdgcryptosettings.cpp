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
#include "xdgportal.h"

namespace {
Logger logger("XdgCryptoSettings");
}  // namespace

XdgCryptoSettings::XdgCryptoSettings()
    : CryptoSettings(),
      XdgPortal(),
      m_metadata(QSettings::UserScope, "mozilla", "vpn_salt") {
  // Check if we can support cryptosettings.
  auto capabilities = QDBusConnection::sessionBus().connectionCapabilities();
  if ((capabilities & QDBusConnection::UnixFileDescriptorPassing) &&
      (getVersion(XDG_PORTAL_SECRET) >= 1)) {
    m_version = EncryptionChachaPolyV1;

    // Save changes to the "token" to the metadata settings file.
    connect(this, &XdgPortal::xdgResponse, this,
            [&](uint replycode, QVariantMap results) {
              if (replycode != 0) {
                return;
              }
              if (results.contains("token")) {
                m_metadata.setValue("token", results.value("token"));
              } else {
                m_metadata.remove("token");
              }
              m_metadata.sync();
            });
  }
}

void XdgCryptoSettings::resetKey() {
  m_metadata.clear();
  m_key.clear();
}

QDBusMessage XdgCryptoSettings::xdgRetrieveSecret(int fd,
                                                  const QVariantMap& options) {
  // Note: QDBusUnixFileDescriptor() will dup the file descriptor we give it,
  // but we need to be very sure that we close all write ends of the pipe. So
  // the point of this wrapper is to ensure that the QDBusUnixFileDescriptor
  // gets destructed and doesn't leave any open file descriptors.
  QDBusMessage msg = QDBusMessage::createMethodCall(
      XDG_PORTAL_SERVICE, XDG_PORTAL_PATH, XDG_PORTAL_SECRET, "RetrieveSecret");
  msg << QVariant::fromValue(QDBusUnixFileDescriptor(fd));
  msg << options;

  return QDBusConnection::sessionBus().call(msg, QDBus::Block);
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

QByteArray XdgCryptoSettings::getKey(const QByteArray& metadata) {
  Q_UNUSED(metadata);

  // Retrieve the key if we don't already have a copy.
  if (m_key.isEmpty()) {
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
    if (m_metadata.contains("token")) {
      options["token"] = m_metadata.value("token");
    }

    QDBusMessage reply = xdgRetrieveSecret(fds[1], options);
    close(fds[1]);  // Close our write end of the pipe right away.
    if (reply.type() != QDBusMessage::ReplyMessage) {
      logger.info() << "Unable to retrieve secret:" << reply.errorMessage();
      return QByteArray();
    } else {
      // We need to rebind our signals if the reply path changed.
      const QVariant& qv = reply.arguments().constFirst();
      setReplyPath(qv.value<QDBusObjectPath>().path());
    }

    // Read the secret.
    m_key = xdgReadSecretFile(fds[0]);
    if (m_key.isEmpty()) {
      // Failed to read the key.
      return QByteArray();
    }
  }

  // Generate a salt if necessary.
  QString salt;
  if (m_metadata.contains("salt")) {
    salt = m_metadata.value("salt").toString();
  } else {
    salt = generateRandomBytes(32).toBase64();
    m_metadata.setValue("salt", QVariant(salt));
    m_metadata.sync();
  }

  // Generate the encryption key.
  HKDF hash(QCryptographicHash::Sha256, salt.toUtf8());
  hash.addData(m_key);
  return hash.result(CRYPTO_SETTINGS_KEY_SIZE);
}

QByteArray XdgCryptoSettings::getMetaData() {
  QJsonObject obj;
  if (m_metadata.contains("salt")) {
    obj["salt"] = QJsonValue(m_metadata.value("salt").toString());
  }
  if (m_metadata.contains("token")) {
    obj["token"] = QJsonValue(m_metadata.value("token").toString());
  }
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}
