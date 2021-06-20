/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSPINGSENDWORKER_H
#define WINDOWSPINGSENDWORKER_H

#include "pingsendworker.h"

#include <QWinEventNotifier>

#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

constexpr WORD WindowsPingPayloadSize = sizeof(WORD);

class WindowsPingSendWorker final : public PingSendWorker {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsPingSendWorker)

 public:
  WindowsPingSendWorker(QObject* parent = nullptr);
  ~WindowsPingSendWorker();

 public slots:
  void sendPing(const QString& destination, const QString& source) override;

 private slots:
  void recvPing();

 private:
  HANDLE m_handle = INVALID_HANDLE_VALUE;
  HANDLE m_event = INVALID_HANDLE_VALUE;
  QWinEventNotifier* m_notifier = nullptr;
  unsigned char m_buffer[sizeof(ICMP_ECHO_REPLY) + WindowsPingPayloadSize + 8];
};

#endif  // WINDOWSPINGSENDWORKER_H
