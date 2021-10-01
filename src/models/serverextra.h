/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVEREXTRA_H
#define SERVEREXTRA_H

#include <QString>

class QJsonObject;

class ServerExtra final {
 public:
  ServerExtra();
  ServerExtra(const ServerExtra& other);
  ServerExtra& operator=(const ServerExtra& other);
  ~ServerExtra();

  [[nodiscard]] bool fromJson(const QJsonObject& obj);

  const QString& publicKey() const { return m_publicKey; }

  const QString& socksName() const { return m_socksName; }

  uint32_t multihopPort() const { return m_multihopPort; }

  bool operator==(const ServerExtra& other) const {
    return m_publicKey == other.m_publicKey;
  }

 private:
  QString m_publicKey;
  QString m_socksName;
  uint32_t m_multihopPort = 0;
};

#endif  // SERVEREXTRA_H
