/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowspingsender.h"
#include "logger.h"
#include "leakdetector.h"
#include "windowscommons.h"

#include <QtEndian>

constexpr const DWORD WINDOWS_ICMP_ECHO_TIMEOUT = 10000;

namespace {
Logger logger({LOG_WINDOWS, LOG_NETWORKING}, "WindowsPingSender");
}

WindowsPingSender::WindowsPingSender(const QHostAddress& source,
                                     QObject* parent)
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

void WindowsPingSender::sendPing(const QHostAddress& dest, quint16 sequence) {
  if (m_handle == INVALID_HANDLE_VALUE) {
    return;
  }
  if (m_event == INVALID_HANDLE_VALUE) {
    return;
  }

  quint32 v4dst = dest.toIPv4Address();
  if (m_source.isNull()) {
    IcmpSendEcho2(m_handle, m_event, nullptr, nullptr,
                  qToBigEndian<quint32>(v4dst), &sequence, sizeof(sequence)),
                  nullptr, m_buffer, sizeof(m_buffer),
                  WINDOWS_ICMP_ECHO_TIMEOUT);
  } else {
    quint32 v4src = m_source.toIPv4Address();
    IcmpSendEcho2Ex(m_handle, m_event, nullptr, nullptr,
                    qToBigEndian<quint32>(v4src), qToBigEndian<quint32>(v4dst),
                    &sequence, sizeof(sequence), nullptr, m_buffer,
                    sizeof(m_buffer), WINDOWS_ICMP_ECHO_TIMEOUT);
  }

  DWORD status = GetLastError();
  if (status != ERROR_IO_PENDING) {
    QString errmsg = WindowsCommons::getErrorMessage();
    logger.error() << "failed to start Code: " << status
                   << " Message: " << errmsg
                   << " dest:" << logger.sensitive(dest.toString());
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
