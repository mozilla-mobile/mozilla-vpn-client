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

WgSessionWorker::WgSessionWorker(WgSessionMacos* session, qintptr socket)
    : QThread(session), m_session(session) {
  MZ_COUNT_CTOR(WgSessionMacos);
  logger.debug() << metaObject()->className() << "created.";

  m_socket = dup(socket);
  m_mtu = IPV6_MMTU;

  int flags = fcntl(m_socket, F_GETFL, 0);
  fcntl(m_socket, F_SETFL, flags & ~O_NONBLOCK);
}

WgSessionWorker::~WgSessionWorker() {
  MZ_COUNT_DTOR(WgSessionMacos);
  logger.debug() << metaObject()->className() << "destroyed.";

  if (m_socket >= 0) {
    close(m_socket);
  }
}

void WgSessionWorker::setMtu(int mtu) {
  logger.debug() << "set mtu:" << mtu;
  m_mtu = mtu;
}

void WgSessionWorker::stop() {
  requestInterruption();
  shutdown(m_socket, SHUT_RD);
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

void WgDecryptWorker::run() {
  QByteArray dgram;
  QByteArray decrypt;

  while (!isInterruptionRequested()) {
    // Resize in case the MTU changed.
    int mtu = m_mtu.loadAcquire();
    dgram.resize(mtu + WgSessionMacos::WG_PACKET_OVERHEAD);
    decrypt.resize(mtu + WgSessionMacos::WG_PACKET_OVERHEAD);

    // Try to read a packet from the network.
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dgram.data());
    int len = recv(m_socket, ptr, dgram.length(), 0);
    if (len < 0) {
      if (errno == EAGAIN) continue;
      if (errno == EINTR) continue;
      logger.debug() << "Recv error:" << strerror(errno);
      return;
    }

    uint8_t* dec = reinterpret_cast<uint8_t*>(decrypt.data());
    auto res = wireguard_read(m_session->m_tunnel, ptr, len, dec, decrypt.size());
    m_session->processResult(res.op, decrypt.first(res.size));
  }
}

void WgMultihopWorker::run() {
  QByteArray dgram;
  QByteArray decrypt;

  while (!isInterruptionRequested()) {
    // Resize in case the MTU changed.
    int mtu = m_mtu.loadAcquire();
    dgram.resize(mtu + WgSessionMacos::WG_PACKET_OVERHEAD);
    decrypt.resize(mtu + WgSessionMacos::WG_PACKET_OVERHEAD);

    // Try to read a packet from the network.
    int len = recv(m_socket, (void*)dgram.data(), dgram.length(), 0);
    if (len < 0) {
      if (errno == EAGAIN) continue;
      if (errno == EINTR) continue;
      logger.debug() << "Recv error:" << strerror(errno);
      return;
    }

    QByteArray packet = m_session->mhopUnwrap(dgram.first(len));
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(packet.constData());
    uint8_t* dec = reinterpret_cast<uint8_t*>(decrypt.data());
    auto res = wireguard_read(m_session->m_tunnel, ptr, packet.size(), dec,
                              decrypt.size());
    m_session->processResult(res.op, decrypt.first(res.size));
  }
}
