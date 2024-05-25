/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgcryptosettings.h"

#include "constants.h"
#include "cryptosettings.h"
#include "hkdf.h"
#include "logger.h"
#include "xdgportal.h"

#include <QScopeGuard>
#include <QSettings>
#include <QtDBus/QtDBus>
#include <unistd.h>

namespace {
Logger logger("XdgCryptoSettings");
}  // namespace

XdgCryptoSettings::XdgCryptoSettings()
    : CryptoSettings(), XdgPortal(),
      m_metadata(QSettings::NativeFormat, QSettings::UserScope, "mozilla",
                 "vpn_salt") {

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
  QDBusMessage msg = QDBusMessage::createMethodCall(XDG_PORTAL_SERVICE,
                                                    XDG_PORTAL_PATH,
                                                    XDG_PORTAL_SECRET,
                                                    "RetrieveSecret");
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
      logger.warning() << "Failed to retrieve encryption key:" << strerror(errno);
      return QByteArray();
    }
    // Retrieved more data.
    data.append(buf, len);
  }

  return data;
}

QByteArray XdgCryptoSettings::getKey() {
  // If the key is known - hash it and return it.
  if (m_key.isEmpty()) {
    auto bus = QDBusConnection::sessionBus();
    auto capabilities = bus.connectionCapabilities();
    if (!(capabilities & QDBusConnection::UnixFileDescriptorPassing)) {
      return QByteArray();
    }

    // Create a pipe to receive the secret.
    int fds[2];
    int err = pipe(fds);
    if (err != 0) {
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
      logger.info() << "Encrypted settings with XDG secrets is not supported";
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

CryptoSettings::Version XdgCryptoSettings::getSupportedVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
