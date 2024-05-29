/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGCRYPTOSETTINGS_H
#define XDGCRYPTOSETTINGS_H

#include <QDBusMessage>

#include "cryptosettings.h"
#include "xdgportal.h"

class XdgCryptoSettings final : public CryptoSettings, public XdgPortal {
 public:
  XdgCryptoSettings();
  virtual ~XdgCryptoSettings() = default;

  void resetKey() override;
  QByteArray getKey(const QByteArray& metadata) override;
  QByteArray getMetaData() override;
  CryptoSettings::Version getSupportedVersion() override { return m_version; };

 private slots:
  void handleResponse(uint code, QVariantMap results);

 private:
  QDBusMessage xdgRetrieveSecret(int fd, const QVariantMap& options);
  static QByteArray xdgReadSecretFile(int fd);

  QString m_token;
  QString m_salt;
  QByteArray m_key;
  CryptoSettings::Version m_version = NoEncryption;
};

#endif  // XDGCRYPTOSETTINGS_H
