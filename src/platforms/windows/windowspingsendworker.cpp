/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowspingsendworker.h"
#include "logger.h"
#include "leakdetector.h"
#include "windowscommons.h"

namespace {
Logger logger({LOG_WINDOWS, LOG_NETWORKING}, "WindowsPingSendWorker");
}

WindowsPingSendWorker::WindowsPingSendWorker(QObject* parent)
    : PingSendWorker(parent) {
  MVPN_COUNT_CTOR(WindowsPingSendWorker);
  m_handle = IcmpCreateFile();
  m_event = CreateEventA(NULL, FALSE, FALSE, NULL);
  memset(m_buffer, 0, sizeof(m_buffer));
}

WindowsPingSendWorker::~WindowsPingSendWorker() {
  MVPN_COUNT_DTOR(WindowsPingSendWorker);
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

void WindowsPingSendWorker::sendPing(const QString& destination,
                                     const QString& source) {
  logger.log() << "start" << destination << "from" << source;

  IN_ADDR dst{};
  IN_ADDR src{};
  if (InetPtonA(AF_INET, destination.toLocal8Bit(), &dst) != 1) {
    emit pingFailed();
    return;
  }
  if (InetPtonA(AF_INET, source.toLocal8Bit(), &src) != 1) {
    emit pingFailed();
    return;
  }
  if (m_handle == INVALID_HANDLE_VALUE) {
    emit pingFailed();
    return;
  }
  if (m_event == INVALID_HANDLE_VALUE) {
    emit pingFailed();
    return;
  }

  constexpr WORD payloadSize = 1;
  unsigned char payload[payloadSize]{42};

  IcmpSendEcho2Ex(m_handle, m_event, nullptr, nullptr, src.S_un.S_addr,
                  dst.S_un.S_addr, payload, payloadSize, nullptr, m_buffer,
                  sizeof(m_buffer), 10000);

  DWORD status = GetLastError();
  if (status != ERROR_IO_PENDING) {
    QString errmsg = WindowsCommons::getErrorMessage();
    logger.log() << "failed to start:" << errmsg;
    emit pingFailed();
    return;
  }

  m_notifier = new QWinEventNotifier(m_event, this);
  QObject::connect(m_notifier, &QWinEventNotifier::activated, this,
                   &WindowsPingSendWorker::recvPing);
}

void WindowsPingSendWorker::recvPing() {
  DWORD replyCount = IcmpParseReplies(m_buffer, sizeof(m_buffer));
  if (replyCount == 0) {
    QString errmsg = WindowsCommons::getErrorMessage();
    logger.log() << "failed with error:" << errmsg;
    emit pingFailed();
    return;
  }

  emit pingSucceeded();
}
