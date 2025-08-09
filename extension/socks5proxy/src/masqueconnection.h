/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MASQUECONNECTION_H
#define MASQUECONNECTION_H

#include "httpconnection.h"

class QIODevice;

class MasqueConnection final : public HttpConnection {
  Q_OBJECT

 public:
  explicit MasqueConnection(QIODevice* socket) : HttpConnection(socket) {};
  ~MasqueConnection() = default;

  // Peek at the socket and determine if this is a MASQUE connection.
  static bool isProxyType(QIODevice* socket);

  void clientProxyRead() override;
  void destProxyRead() override;

 private slots:
  void onHostnameResolved(const QHostAddress& addr);
  void onHttpGet();

 private:
  // QUIC variable integer handling.
  bool readVarInt(quint64& value);
  static void putVarInt(quint64 value, QByteArray& buffer);

  // RFC 9297 HTTP datagram and capsule protocol handling.
  void handleCapsule(quint64 type, const QByteArray& data);

  quint64 m_capsuleType = 0;
  quint64 m_capsuleLength = 0;
  QByteArray m_capsuleBuffer;
  bool m_capsuleDrop = false;
};

#endif  // MASQUECONNECTION_H
