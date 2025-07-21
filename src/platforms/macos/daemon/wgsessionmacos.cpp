/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionmacos.h"

#include <fcntl.h>
#include <net/if_utun.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/sys_domain.h>
#include <sys/uio.h>
#include <unistd.h>

#include <QDateTime>
#include <QMetaMethod>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QUdpSocket>
#include <QtEndian>

#include "daemon/wireguardutils.h"
#include "interfaceconfig.h"
#include "leakdetector.h"
#include "logger.h"
#include "wgsessionworker.h"

extern "C" {
#include "wireguard_ffi.h"
}

namespace {
Logger logger("WgSessionMacos");
};  // namespace

constexpr const int WG_SESSION_TICK_INTERVAL = 100;
constexpr const int WG_DEFRAG_TIMEOUT = 1000;
constexpr const int WG_MAX_HANDSHAKE_SIZE = 148;

WgSessionMacos::WgSessionMacos(const InterfaceConfig& config, QObject* parent)
    : QObject(parent), m_config(config) {
  MZ_COUNT_CTOR(WgSessionMacos);
  logger.debug() << "WgSessionMacos created.";

  // Allocate a new tunnel
  quint32 index = QRandomGenerator::system()->bounded(1 << 24);
  m_tunnel = new_tunnel(config.m_privateKey.toUtf8().constData(),
                        config.m_serverPublicKey.toLocal8Bit().constData(),
                        nullptr, WG_KEEPALIVE_PERIOD, index);
  connect(&m_timer, &QTimer::timeout, this, &WgSessionMacos::timeout);
  m_timer.setSingleShot(false);
  m_timer.start(WG_SESSION_TICK_INTERVAL);

  // If multihop is used - allocate some random UDP port and IP settings.
  m_innerPort = QRandomGenerator::system()->bounded(49152, 65535);
  m_serverPort = m_config.m_serverPort;
  m_innerTTL = 64;
  m_innerIpv4.setAddress(m_config.m_deviceIpv4Address.section('/', 0, 0));
  m_innerIpv6.setAddress(m_config.m_deviceIpv6Address.section('/', 0, 0));
  m_serverIpv4.setAddress(m_config.m_serverIpv4AddrIn);
  m_serverIpv6.setAddress(m_config.m_serverIpv6AddrIn);

  m_tunmtu = IPV6_MMTU;
}

WgSessionMacos::~WgSessionMacos() {
  MZ_COUNT_DTOR(WgSessionMacos);
  logger.debug() << "WgSessionMacos destroyed.";
  stopWorkers();

  if (m_tunnel) {
    tunnel_free(m_tunnel);
  }
  if (m_netSocket >= 0) {
    close(m_netSocket);
  }
}

void WgSessionMacos::processResult(int op, const QByteArray& buf) const {
  switch (op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
        tunWrite(m_netSocket, mhopHeader(buf), buf);
      } else {
        //logger.debug() << name() << "output:" << QByteArray(buf.toBase64());
        send(m_netSocket, buf.constData(), buf.length(), MSG_DONTWAIT);
      }
      break;

    case WIREGUARD_ERROR:
      logger.warning() << "Wireguard error";
      break;

    case WRITE_TO_TUNNEL_IPV4:
      [[fallthrough]];
    case WRITE_TO_TUNNEL_IPV6:
      tunWrite(m_tunSocket, buf);
      break;
  }
}

void WgSessionMacos::timeout() {
  QByteArray buffer;
  buffer.resize(WG_MAX_HANDSHAKE_SIZE);

  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_tick(m_tunnel, bufptr, buffer.length());
  processResult(result.op, buffer.first(result.size));

  // Process defrag timeouts.
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  for (quint16 ident : m_defrag.keys()) {
    if (m_defrag[ident].m_timeout < now) {
      m_defrag.remove(ident);
    }
  }
}

void WgSessionMacos::renegotiate() {
  QByteArray output;
  output.resize(WG_MAX_HANDSHAKE_SIZE);

  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto res = wireguard_force_handshake(m_tunnel, outptr, output.size());

  processResult(res.op, output.first(res.size));
}

WireguardUtils::PeerStatus WgSessionMacos::status() const {
  WireguardUtils::PeerStatus result(m_config.m_serverPublicKey);

  auto wgStats = wireguard_stats(m_tunnel);
  result.m_rxBytes = wgStats.rx_bytes;
  result.m_txBytes = wgStats.tx_bytes;
  if (wgStats.time_since_last_handshake >= 0) {
    qint64 elapsed = wgStats.time_since_last_handshake * 1000;
    result.m_handshake = QDateTime::currentMSecsSinceEpoch() - elapsed;
  }
  return result;
}

quint16 WgSessionMacos::inetChecksum(const void* data, size_t len,
                                     quint32 seed) {
  const quint16* ptr = reinterpret_cast<const quint16*>(data);
  quint32 chksum = seed;
  for (const quint16* end = ptr + len / 2; ptr < end; ptr++) {
    chksum += qFromBigEndian(*ptr);
  }
  // This can overflow at most once, so we just do it twice and we're good.
  chksum = (chksum >> 16) + (chksum & 0xffff);
  chksum = (chksum >> 16) + (chksum & 0xffff);
  return htons(~chksum);
}

quint16 WgSessionMacos::udpChecksum(const QHostAddress& source,
                                    const QHostAddress& dest, quint16 sport,
                                    quint16 dport, const QByteArray& payload) {
  quint32 src4 = source.toIPv4Address();
  quint32 dst4 = dest.toIPv4Address();
  quint32 seed = 0;

  // The UDP pseudo-header
  seed += (src4 >> 16) + (src4 & 0xffff);
  seed += (dst4 >> 16) + (dst4 & 0xffff);
  seed += IPPROTO_UDP;
  seed += payload.length() + 8;

  // The UDP header
  seed += sport;
  seed += dport;
  seed += payload.length() + 8;

  // And the datagram's payload.
  return inetChecksum(payload.constData(), payload.length(), seed);
}

struct ipv4header {
  uint8_t ihl;
  uint8_t tos;
  quint16 length;
  quint16 ident;
  quint16 frag;
  uint8_t ttl;
  uint8_t proto;
  quint16 checksum;
  quint32 source;
  quint32 dest;
};

QByteArray WgSessionMacos::mhopHeader(const QByteArray& packet) const {
  uint16_t udpcksum =
      udpChecksum(m_innerIpv4, m_serverIpv4, m_innerPort, m_serverPort, packet);
  uint16_t udphdr[4] = {qToBigEndian(m_innerPort), qToBigEndian(m_serverPort),
                        qToBigEndian<quint16>(packet.length() + 8), udpcksum};

  quint16 tlen = packet.length() + sizeof(struct ipv4header) + sizeof(udphdr);
  struct ipv4header header = {
      .ihl = 0x45,
      .length = qToBigEndian(tlen),
      .ttl = m_innerTTL,
      .proto = IPPROTO_UDP,
      .checksum = 0,
      .source = qToBigEndian(m_innerIpv4.toIPv4Address()),
      .dest = qToBigEndian(m_serverIpv4.toIPv4Address()),
  };
  header.checksum = inetChecksum(&header, sizeof(header));

  QByteArray result;
  result.reserve(sizeof(header) + sizeof(udphdr));
  result.append((const char*)&header, sizeof(header));
  result.append((const char*)udphdr, sizeof(udphdr));
  return result;
}

QByteArray WgSessionMacos::mhopUnwrap(const QByteArray& packet) {
  if (packet.length() <= sizeof(quint32)) {
    return QByteArray();
  }

  quint32 header = qFromBigEndian<quint32>(packet.constData());
  if (header == AF_INET) {
    return mhopInputV4(packet.sliced(sizeof(header)));
  } else if (header == AF_INET6) {
    return mhopInputV6(packet.sliced(sizeof(header)));
  } else {
    return QByteArray();
  }
}

QByteArray WgSessionMacos::mhopInputV4(const QByteArray& packet) {
  // Parse the IPv4 header
  auto header = reinterpret_cast<const struct ipv4header*>(packet.constData());
  quint16 hlen = (header->ihl & 0xF) * 4;
  if ((header->ihl >> 4) != 4) {
    return QByteArray();
  }
  if ((hlen < sizeof(struct ipv4header)) || (hlen > packet.length())) {
    return QByteArray();
  }

  // Validate the header.
  if ((qFromBigEndian(header->source) != m_serverIpv4.toIPv4Address()) ||
      (qFromBigEndian(header->dest) != m_innerIpv4.toIPv4Address()) ||
      (header->proto != IPPROTO_UDP) || (header->ttl == 0) ||
      inetChecksum(header, sizeof(struct ipv4header)) != 0x0000) {
    return QByteArray();
  }

  // Handle IPv4 defragmentation
  QByteArray dgram = packet.sliced(hlen);
  if (header->frag & qToBigEndian<quint16>(0x3fff)) {
    dgram = mhopDefragV4(header, dgram);
    if (dgram.isEmpty()) {
      return QByteArray();
    }
  }

  // Process the UDP header
  return mhopInputUDP(m_serverIpv4, m_innerIpv4, dgram);
}

QByteArray WgSessionMacos::mhopDefragV4(const struct ipv4header* header,
                                        const QByteArray& dgram) {
  quint16 flags = qFromBigEndian(header->frag);
  quint16 ident = qFromBigEndian(header->ident);
  quint16 offset = (flags & 0x1fff) * 8;
  bool moreFragments = (flags & (1 << 13)) != 0;

  // Except for the last fragment, every chunk must be 8-byte aligned.
  if (moreFragments && (dgram.length() % 8)) {
    return QByteArray();
  }

  // Get the defrag state, or create a new one.
  Ipv4DefragState& state = m_defrag[ident];
  if (offset == 0) {
    state.m_timeout = QDateTime::currentMSecsSinceEpoch() + WG_DEFRAG_TIMEOUT;
    state.m_buffer = dgram;
  } else if (state.m_buffer.length() != offset) {
    // Fragment arrived out of order.
    return QByteArray();
  } else {
    state.m_buffer.append(dgram);
  }

  if (moreFragments) {
    // More fragments are expected.
    return QByteArray();
  }
  QByteArray result(state.m_buffer);
  m_defrag.remove(ident);
  return result;
}

QByteArray WgSessionMacos::mhopInputV6(const QByteArray& packet) {
  // TODO: Implement Me!
  return QByteArray();
}

QByteArray WgSessionMacos::mhopInputUDP(const QHostAddress& src,
                                        const QHostAddress& dst,
                                        const QByteArray& dgram) {
  const quint16* hdr = reinterpret_cast<const quint16*>(dgram.constData());
  if ((dgram.length() < 8) || (hdr[0] != htons(m_serverPort)) ||
      (hdr[1] != htons(m_innerPort)) || (htons(hdr[2]) > dgram.length())) {
    logger.debug() << "mhop drop udp:" << dgram.toHex();
    return QByteArray();
  }

  QByteArray data = dgram.sliced(8);
  if (hdr[3] != 0x0000) {
    // Validate the checksum
    quint16 cksum = udpChecksum(src, dst, htons(hdr[0]), htons(hdr[1]), data);
    if (hdr[3] != cksum) {
      logger.debug() << "mhop drop cksum:" << dgram.toHex();
      return QByteArray();
    }
  }

  return data;
}

bool WgSessionMacos::start(const struct sockaddr* addr, int len) {
  Q_ASSERT(m_netSocket < 0);

  qintptr sock = socket(addr->sa_family, SOCK_DGRAM, 0);
  if (sock < 0) {
    logger.warning() << "Socket creation failed:" << strerror(errno);
    return false;
  }

  if (::connect(sock, addr, len) < 0) {
    logger.warning() << "Socket connect failed:" << strerror(errno);
    close(sock);
    return false;
  }

  start(sock);
  return true;
}

void WgSessionMacos::start(qintptr sd) {
  int flags = fcntl(sd, F_GETFL, 0);
  fcntl(sd, F_SETFL, flags | O_NONBLOCK);

  m_netSocket = sd;

  // Start the decryption worker.
  if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
    startWorker(new WgMultihopWorker(this, sd));
  } else {
    startWorker(new WgDecryptWorker(this, sd));
  }

  renegotiate();
}

void WgSessionMacos::startWorker(WgSessionWorker* worker) {
  logger.info() << "starting " << worker->metaObject()->className();

  connect(worker, &QThread::started, this, [this](){ m_workerWaitCount++; });
  connect(worker, &QThread::finished, this, [this](){ m_workerWaitGroup.release(); }, Qt::DirectConnection);
  m_workers.append(worker);

  worker->setMtu(m_tunmtu);
  worker->start();
}

void WgSessionMacos::stopWorkers() {
  constexpr int WORKER_SHUTDOWN_TIMEOUT = 3000;
  logger.info() << "Stopping" << m_workerWaitCount << "workers";

  // Request all the workers to stop and wait for them to finish.
  for (WgSessionWorker* worker : m_workers) {
    worker->stop();
  }
  m_workerWaitGroup.tryAcquire(m_workerWaitCount, WORKER_SHUTDOWN_TIMEOUT);

  // Delete the threads.
  for (WgSessionWorker* worker : m_workers) {
    if (worker->isRunning()) {
      logger.warning() << worker->metaObject()->className() << "still running";
      worker->terminate();
      worker->wait(WORKER_SHUTDOWN_TIMEOUT);
    }
    delete worker;
  }

  // Reset our state.
  m_workers.clear();
  m_workerWaitCount = 0;
  m_workerWaitGroup.tryAcquire(m_workerWaitGroup.available());
}

void WgSessionMacos::setTunSocket(qintptr sd) {
  int flags = fcntl(sd, F_GETFL, 0);
  fcntl(sd, F_SETFL, flags | O_NONBLOCK);

  m_tunSocket = sd;

  // Start the encryption worker.
  startWorker(new WgEncryptWorker(this, sd));
}

void WgSessionMacos::setMtu(int mtu) {
  if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
    mtu -= WG_MTU_OVERHEAD;
  }
  if (mtu < IPV6_MMTU) {
    mtu = IPV6_MMTU;
  }
  m_tunmtu = mtu;

  for (WgSessionWorker* worker : m_workers) {
    worker->setMtu(mtu);
  }

  // Set the MTU if it's a utun device.
  struct ifreq ifr;
  socklen_t ifnamesize = sizeof(ifr.ifr_name);
  int err = getsockopt(m_tunSocket, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifr.ifr_name,
                       &ifnamesize);

  ifr.ifr_mtu = m_tunmtu;
  if ((err >= 0) && (ioctl(m_tunSocket, SIOCSIFMTU, &ifr) != 0)) {
    logger.error() << "Failed to set MTU:" << strerror(errno);
  }
}

void WgSessionMacos::tunWrite(qintptr fd, const QByteArray& packet, const QByteArray& append) const {
  //logger.debug() << name() << "decrypt:" << QString(packet.toBase64());
  quint32 family = ((packet.at(0) >> 4) == 4) ? AF_INET : AF_INET6;
  quint32 header = qToBigEndian<quint32>(family);

  struct iovec iov[3];
  int count = 2;
  iov[0].iov_base = &header;
  iov[0].iov_len = sizeof(header);
  iov[1].iov_base = (void*)packet.constData();
  iov[1].iov_len = packet.length();
  if (!append.isEmpty()) {
    iov[2].iov_base = (void*)append.constData();
    iov[2].iov_len = append.length();
    count++;
  }

  writev(fd, iov, count);
}
