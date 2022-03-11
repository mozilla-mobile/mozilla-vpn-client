/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DNSPINGSENDER_H
#define DNSPINGSENDER_H

#include "pingsender.h"

#include <QUdpSocket>

class DnsPingSender final : public PingSender {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DnsPingSender)

 public:
  DnsPingSender(const QString& source, QObject* parent = nullptr);
  ~DnsPingSender();

  void sendPing(const QString& dest, quint16 sequence) override;

 private:
  void readData();

 private:
  QUdpSocket m_socket;
};

#endif  // DNSPINGSENDER_H
