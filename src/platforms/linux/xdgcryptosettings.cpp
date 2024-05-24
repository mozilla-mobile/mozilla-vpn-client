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
#include <QtDBus/QtDBus>
#include <unistd.h>

namespace {
Logger logger("XdgCryptoSettings");
}  // namespace

XdgCryptoSettings::XdgCryptoSettings() : CryptoSettings(), XdgPortal() {
  logger.debug() << "XdgCryptoSettings created";
}

void XdgCryptoSettings::resetKey() {
  // TODO: Implement Me!
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

bool XdgCryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  auto bus = QDBusConnection::sessionBus();
  auto capabilities = bus.connectionCapabilities();
  if (!(capabilities & QDBusConnection::UnixFileDescriptorPassing)) {
    return false;
  }

  // Create a pipe to receive the secret.
  int fds[2];
  int err = pipe(fds);
  if (err != 0) {
    return false;
  }
  auto guard = qScopeGuard([&] { close(fds[0]); });

  // Request the secret.
  QVariantMap options;
  options["handle_token"] = QVariant(token());

  QDBusMessage reply = xdgRetrieveSecret(fds[1], options);
  close(fds[1]);  // Close our write end of the pipe right away.
  if (reply.type() != QDBusMessage::ReplyMessage) {
    logger.info() << "Encrypted settings with XDG secrets is not supported";
    return false;
  } else {
    // We need to rebind our signals if the reply path changed.
    QVariant qv = reply.arguments().first();
    QString path = qv.value<QDBusObjectPath>().path();
    logger.debug() << "Expecting XDG response at:" << path;
    setReplyPath(path);
  }

  // Hash the returned secret into a usable key.
  HKDF hash(QCryptographicHash::Sha256);
  int keysize = 0;
  for (;;) {
    char buf[PIPE_BUF];
    int len = read(fds[0], buf, sizeof(buf));
    if (len > 0) {
      // Retrieved more data.
      keysize += len;
      hash.addData(buf, len);
      continue;
    }
    else if (len == 0) {
      // End-of-file. Generate the encryption key.
      QByteArray result = hash.result(CRYPTO_SETTINGS_KEY_SIZE);
      memcpy(key, result.constData(), CRYPTO_SETTINGS_KEY_SIZE);
      return true;
    }
    else {
      // An error occured.
      logger.warning() << "Failed to retrieve encryption key:" << strerror(errno);
      return false;
    }
  }
}

CryptoSettings::Version XdgCryptoSettings::getSupportedVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
