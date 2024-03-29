/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowspingsender.h"

#include <ranges>
#include <mutex>

#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <winternl.h>

// Note: This important must come after the previous three.
// clang-format off
#include <IcmpAPI.h>
// clang-format on

#include <QtEndian>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowsutils.h"
#include "windowscommons.h"

#pragma comment(lib, "Ws2_32")

constexpr WORD WindowsPingPayloadSize = sizeof(quint16);
constexpr size_t ICMP_ERR_SIZE = 8;
constexpr size_t MinimumReplyBufferSize = sizeof(ICMP_ECHO_REPLY) +
                                   WindowsPingPayloadSize + ICMP_ERR_SIZE +
                                   sizeof(IO_STATUS_BLOCK);

struct WindowsPingSenderPrivate {
  HANDLE m_handle;
  HANDLE m_event;
  std::array<ICMP_ECHO_REPLY, 2> m_replyBuffer;
};
static_assert(sizeof(WindowsPingSenderPrivate::m_replyBuffer) > MinimumReplyBufferSize,
              "Fulfills the minimum size requirements");



namespace {
Logger logger("WindowsPingSender");

std::mutex io_mutex;

}

static DWORD icmpCleanupHelper(LPVOID data) {
  struct WindowsPingSenderPrivate* p = (struct WindowsPingSenderPrivate*)data;
  if (p->m_event != INVALID_HANDLE_VALUE) {
    CloseHandle(p->m_event);
  }
  if (p->m_handle != INVALID_HANDLE_VALUE) {
    IcmpCloseHandle(p->m_handle);
  }
  delete p;
  return 0;
}

WindowsPingSender::WindowsPingSender(const QHostAddress& source,
                                     QObject* parent)
    : PingSender(parent) {
  MZ_COUNT_CTOR(WindowsPingSender);
  m_source = source;
  m_private = new struct WindowsPingSenderPrivate;
  m_private->m_handle = IcmpCreateFile();
  m_private->m_event = CreateEventA(NULL, FALSE, FALSE, NULL);

  m_notifier = new QWinEventNotifier(m_private->m_event, this);
  QObject::connect(m_notifier, &QWinEventNotifier::activated, this,
                   &WindowsPingSender::pingEventReady);

  m_private->m_replyBuffer.fill({});
}

WindowsPingSender::~WindowsPingSender() {
  MZ_COUNT_DTOR(WindowsPingSender);
  if (m_notifier) {
    delete m_notifier;
  }
  // Closing the ICMP handle can hang if there are lost ping replies. Moving
  // the cleanup into a separate thread avoids deadlocking the application.
  HANDLE h = CreateThread(NULL, 0, icmpCleanupHelper, m_private, 0, NULL);
  if (h == NULL) {
    icmpCleanupHelper(m_private);
  } else {
    CloseHandle(h);
  }
}

void WindowsPingSender::sendPing(const QHostAddress& dest, quint16 sequence) {
  if (m_private->m_handle == INVALID_HANDLE_VALUE) {
    return;
  }
  if (m_private->m_event == INVALID_HANDLE_VALUE) {
    return;
  }

  std::scoped_lock lock(io_mutex);

  quint32 v4dst = dest.toIPv4Address();
  if (m_source.isNull()) {
    IcmpSendEcho2(m_private->m_handle,           // IcmpHandle,
                  m_private->m_event,            // Event
                  nullptr,                       // ApcRoutine
                  nullptr,                       // ApcContext
                  qToBigEndian<quint32>(v4dst),  // DestinationAddress
                  &sequence,                     // RequestData
                  sizeof(sequence),              // RequestSize
                  nullptr,                       // RequestOptions
                  &m_private->m_replyBuffer,     // [OUT] ReplyBuffer
                  sizeof(m_private->m_replyBuffer),  // ReplySize
                  10000                          // Timeout
    );
  } else {
    quint32 v4src = m_source.toIPv4Address();
    IcmpSendEcho2Ex(m_private->m_handle,           // IcmpHandle
                    m_private->m_event,            // Event
                    nullptr,                       // ApcRoutine
                    nullptr,                       // ApcContext
                    qToBigEndian<quint32>(v4src),  // SourceAddress
                    qToBigEndian<quint32>(v4dst),  // DestinationAddress
                    &sequence,                     // RequestData
                    sizeof(sequence),              // RequestSize
                    nullptr,                       // RequestOptions
                    &m_private->m_replyBuffer,         // [OUT] ReplyBuffer
                    sizeof(m_private->m_replyBuffer),   // ReplySize
                    10000                          // Timeout
    );
  }

  DWORD status = GetLastError();
  if (status != ERROR_IO_PENDING) {
    QString errmsg = WindowsUtils::getErrorMessage();
    logger.error() << "failed to start Code: " << status
                   << " Message: " << errmsg
                   << " dest:" << logger.sensitive(dest.toString());
  }
}

void WindowsPingSender::pingEventReady() {
  std::scoped_lock lock(io_mutex);
  const auto guard = qScopeGuard([this](){ 
      m_private->m_replyBuffer.fill({});
  });
  // Move ICMP State, to m_buffer is reset to 0;
  auto state = m_private->m_replyBuffer; 
  m_private->m_replyBuffer.fill({});

  DWORD replyCount = IcmpParseReplies(&state, sizeof(state));
  if (replyCount == 0) {
    DWORD error = GetLastError();
    if (error == IP_REQ_TIMED_OUT) {
      return;
    }
    QString errmsg = WindowsUtils::getErrorMessage();
    logger.error() << "No ping reply. Code: " << error
                   << " Message: " << errmsg;
    return;
  }
  for (auto const& reply :
       state | std::ranges::views::take(replyCount)) {
    if (reply.Data == nullptr) {
      continue;
    }
    quint16 sequence = *(quint16*)reply.Data;
    emit recvPing(sequence);
  }
}
