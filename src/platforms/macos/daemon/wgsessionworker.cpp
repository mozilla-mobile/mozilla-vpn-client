/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionworker.h"

#include <fcntl.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <signal.h>
#include <unistd.h>

#include "leakdetector.h"
#include "logger.h"
#include "wgsessionmacos.h"

extern "C" {
#include "wireguard_ffi.h"
}

namespace {
Logger logger("WgSessionWorker");
};  // namespace

WgEncryptWorker::WgEncryptWorker(WgSessionMacos* session, qintptr socket)
    : QThread(session), m_session(session) {
  MZ_COUNT_CTOR(WgSessionMacos);
  m_socket = dup(socket);
  m_mtu = IPV6_MMTU;

  int flags = fcntl(m_socket, F_GETFL, 0);
  fcntl(m_socket, F_SETFL, flags & ~O_NONBLOCK);
}

WgEncryptWorker::~WgEncryptWorker() {
  MZ_COUNT_DTOR(WgSessionMacos);
  if (m_socket >= 0) {
    close(m_socket);
  }
}

void WgEncryptWorker::run() {
  quint32 header = 0;
  QByteArray packet;
  QByteArray encrypt;

  while (!isInterruptionRequested()) {
    // Resize in case the MTU changed.
    int mtu = m_mtu.loadAcquire();
    packet.resize(mtu + 16);
    encrypt.resize(mtu + WgSessionMacos::WG_PACKET_OVERHEAD);

    struct iovec iov[2];
    iov[0].iov_base = &header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_len = mtu;
    iov[1].iov_base = (void*)packet.data();

    // Try to read a packet from the tunnel.
    int len = readv(m_socket, iov, sizeof(iov) / sizeof(struct iovec));
    if (len < 0) {
      if (errno == EAGAIN) continue;
      if (errno == EINTR) continue;
      logger.debug() << "Tunnel error:" << strerror(errno);
      return;
    }
    int pktlen = len - sizeof(header);
    if ((pktlen < 0) || (pktlen > mtu)) {
      continue;
    }

    // I think there is a small bug in boringtun to do with message padding.
    // The wireguard protocol states that the encapsulated packet must first be
    // padded out to a multiple of 16 bytes in length, but boringtun does no
    // such padding during encryption. So let's do it manually ourself.
    int tail = pktlen % 16;
    if (tail) {
      int padsz = 16 - tail;
      memset(packet.data() + pktlen, 0, padsz);
      pktlen += padsz;
    }

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(packet.constData());
    uint8_t* enc = reinterpret_cast<uint8_t*>(encrypt.data());
    auto res = wireguard_write(m_session->m_tunnel, ptr, pktlen, enc,
                                  encrypt.size());

    m_session->processResult(res.op, encrypt.first(res.size));
  }
}

void WgEncryptWorker::shutdown() {
  requestInterruption();
  ::shutdown(m_socket, SHUT_RD);
}

void WgDecryptWorker::run() {
  QByteArray output;
  output.resize(m_packet.size());

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(m_packet.constData());
  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto res = wireguard_read(m_session->m_tunnel, ptr, m_packet.size(), outptr,
                            output.size());
  m_session->processResult(res.op, output.first(res.size));
}
