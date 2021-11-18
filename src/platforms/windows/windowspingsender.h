/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSPINGSENDER_H
#define WINDOWSPINGSENDER_H

#pragma comment(lib, "Ws2_32")

#include "pingsender.h"

#include <QMap>
#include <QWinEventNotifier>

#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

constexpr WORD WindowsPingPayloadSize = sizeof(quint16);

struct WindowsPingContext {
  HANDLE event;
  quint16 sequence;
};

class WindowsPingSender final : public PingSender {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsPingSender)

 public:
  WindowsPingSender(const QString& source, QObject* parent = nullptr);
  ~WindowsPingSender();

  void sendPing(const QString& destination, quint16 sequence) override;

 private slots:
  void pingEventReady();

 private:
  QString m_source;
  HANDLE m_handle = INVALID_HANDLE_VALUE;
  HANDLE m_event = INVALID_HANDLE_VALUE;
  QWinEventNotifier* m_notifier = nullptr;

  unsigned char m_buffer[sizeof(ICMP_ECHO_REPLY) + WindowsPingPayloadSize + 8];
};

#endif  // WINDOWSPINGSENDER_H
