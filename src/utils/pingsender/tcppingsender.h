/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TCPPINGSENDER_H
#define TCPPINGSENDER_H

#include "pingsender.h"

class TcpPingSender final : public PingSender {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TcpPingSender)

 public:
  TcpPingSender(const QHostAddress& source, quint16 port = 80,
                QObject* parent = nullptr);
  ~TcpPingSender();

  void sendPing(const QHostAddress& dest, quint16 sequence) override;

 private:
  QHostAddress m_source;
  quint16 m_port;
};

#endif  // DNSPINGSENDER_H
