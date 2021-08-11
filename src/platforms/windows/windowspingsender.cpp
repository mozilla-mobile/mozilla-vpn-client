/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowspingsender.h"
#include "logger.h"
#include "leakdetector.h"
#include "windowscommons.h"

namespace {
Logger logger({LOG_WINDOWS, LOG_NETWORKING}, "WindowsPingSender");
}

WindowsPingSender::WindowsPingSender(const QString& source, QObject* parent)
    : PingSender(parent) {
  MVPN_COUNT_CTOR(WindowsPingSender);
  m_source = source;
  m_handle = IcmpCreateFile();
  m_event = CreateEventA(NULL, FALSE, FALSE, NULL);

  m_notifier = new QWinEventNotifier(m_event, this);
  QObject::connect(m_notifier, &QWinEventNotifier::activated, this,
                   &WindowsPingSender::pingEventReady);

  memset(m_buffer, 0, sizeof(m_buffer));
}

WindowsPingSender::~WindowsPingSender() {
  MVPN_COUNT_DTOR(WindowsPingSender);
  if (m_notifier) {
    delete m_notifier;
  }
  if (m_event != INVALID_HANDLE_VALUE) {
    CloseHandle(m_event);
  }
  if (m_handle != INVALID_HANDLE_VALUE) {
    IcmpCloseHandle(m_handle);
  }
}

void WindowsPingSender::sendPing(const QString& dest, quint16 sequence) {
  IN_ADDR dst{};
  IN_ADDR src{};
  if (InetPtonA(AF_INET, dest.toLocal8Bit(), &dst) != 1) {
    return;
  }
  if (InetPtonA(AF_INET, m_source.toLocal8Bit(), &src) != 1) {
    return;
  }
  if (m_handle == INVALID_HANDLE_VALUE) {
    return;
  }
  if (m_event == INVALID_HANDLE_VALUE) {
    return;
  }

  IcmpSendEcho2Ex(m_handle, m_event, nullptr, nullptr, src.S_un.S_addr,
                  dst.S_un.S_addr, &sequence, sizeof(sequence), nullptr,
                  m_buffer, sizeof(m_buffer), 10000);

  DWORD status = GetLastError();
  if (status != ERROR_IO_PENDING) {
    QString errmsg = WindowsCommons::getErrorMessage();
    logger.error() << "failed to start:" << errmsg;
  }
}

void WindowsPingSender::pingEventReady() {
  DWORD replyCount = IcmpParseReplies(m_buffer, sizeof(m_buffer));
  if (replyCount == 0) {
    DWORD error = GetLastError();
    if (error == IP_REQ_TIMED_OUT) {
      return;
    }
    QString errmsg = WindowsCommons::getErrorMessage();
    logger.error() << "failed with error:" << errmsg;
    return;
  }

  const ICMP_ECHO_REPLY* replies = (const ICMP_ECHO_REPLY*)m_buffer;
  for (DWORD i = 0; i < replyCount; i++) {
    if (replies[i].DataSize < sizeof(quint16)) {
      continue;
    }
    quint16 sequence;
    memcpy(&sequence, replies[i].Data, sizeof(quint16));
    emit recvPing(sequence);
  }
}
