/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowspingsendworker.h"
#include "logger.h"
#include "leakdetector.h"

#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

namespace {
Logger logger({LOG_WINDOWS, LOG_NETWORKING}, "WindowsPingSendWorker");
}

WindowsPingSendWorker::WindowsPingSendWorker() {
  MVPN_COUNT_CTOR(WindowsPingSendWorker);
}

WindowsPingSendWorker::~WindowsPingSendWorker() {
  MVPN_COUNT_DTOR(WindowsPingSendWorker);
}

void WindowsPingSendWorker::sendPing(const QString& destination,
                                     const QString& source) {
  logger.log() << "WindowsPingSendWorker - start" << destination << "from"
               << source;

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

  HANDLE icmpHandle = IcmpCreateFile();
  if (icmpHandle == INVALID_HANDLE_VALUE) {
    emit pingFailed();
    return;
  }

  constexpr WORD payloadSize = 1;
  unsigned char payload[payloadSize]{42};

  constexpr DWORD replyBufferSize = sizeof(ICMP_ECHO_REPLY) + payloadSize + 8;
  unsigned char replyBuffer[replyBufferSize]{};

  DWORD replyCount =
      IcmpSendEcho2Ex(icmpHandle, INVALID_HANDLE_VALUE, nullptr, nullptr,
                      src.S_un.S_addr, dst.S_un.S_addr, payload, payloadSize,
                      nullptr, replyBuffer, replyBufferSize, 10000);
  IcmpCloseHandle(icmpHandle);

  if (replyCount == 0) {
    emit pingFailed();
    return;
  }

  emit pingSucceeded();
}
