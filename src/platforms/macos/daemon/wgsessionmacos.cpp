/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionmacos.h"

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

extern "C" {
#include "wireguard_ffi.h"
}

namespace {
Logger logger("WgSessionMacos");
};  // namespace

constexpr const int WG_SESSION_TICK_INTERVAL = 100;
constexpr const int WG_DEFRAG_TIMEOUT = 1000;
constexpr const int WG_PACKET_OVERHEAD = 32;
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
  if (m_tunnel) {
    tunnel_free(m_tunnel);
  }
  if (m_netSocket >= 0) {
    close(m_netSocket);
  }
}

void WgSessionMacos::processResult(int op, const QByteArray& buf) {
  switch (op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
        netWrite(mhopEncapsulate(buf));
      } else {
        netWrite(buf);
      }
      break;

    case WIREGUARD_ERROR:
      logger.warning() << "Wireguard error";
      break;

    case WRITE_TO_TUNNEL_IPV4:
      tunWrite(buf, AF_INET);
      break;

    case WRITE_TO_TUNNEL_IPV6:
      tunWrite(buf, AF_INET6);
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

void WgSessionMacos::encrypt(const QByteArray& pkt) {
  QByteArray output;
  output.resize(pkt.size() + WG_PACKET_OVERHEAD);

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(pkt.constData());
  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto res = wireguard_write(m_tunnel, ptr, pkt.size(), outptr, output.size());

  processResult(res.op, output.first(res.size));
}

void WgSessionMacos::netInput(const QByteArray& pkt) {
  logger.debug() << "net input:" << QString(pkt.toBase64());
  QByteArray output;
  output.resize(pkt.size());

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(pkt.constData());
  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto res = wireguard_read(m_tunnel, ptr, pkt.size(), outptr, output.size());

  processResult(res.op, output.first(res.size));
}

void WgSessionMacos::mhopInput(const QByteArray& pkt) {
  quint8 version = (pkt[0] >> 4);
  if (version == 4) {
    mhopInputV4(pkt);
  } else if (version == 6) {
    mhopInputV6(pkt);
  }
}

WireguardUtils::PeerStatus WgSessionMacos::status() const {
  WireguardUtils::PeerStatus result(m_config.m_serverPublicKey);

  auto wgStats = wireguard_stats(m_tunnel);
  result.m_rxBytes = wgStats.rx_bytes;
  result.m_txBytes = wgStats.tx_bytes;
  if (wgStats.time_since_last_handshake > 0) {
    qint64 elapsed = wgStats.time_since_last_handshake;
    result.m_handshake = (QDateTime::currentSecsSinceEpoch() - elapsed) * 1000;
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

QByteArray WgSessionMacos::mhopEncapsulate(const QByteArray& packet) {
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
      .source = qToBigEndian(m_innerIpv4.toIPv4Address()),
      .dest = qToBigEndian(m_serverIpv4.toIPv4Address()),
  };
  // Compute the checksums.
  header.checksum = inetChecksum(&header, sizeof(header));

  // Return the encapsulated packet.
  QByteArray result;
  result.reserve(sizeof(header) + sizeof(udphdr) + packet.length());
  result.append(reinterpret_cast<char*>(&header), sizeof(header));
  result.append(reinterpret_cast<char*>(udphdr), sizeof(udphdr));
  result.append(packet);
  return result;
}

void WgSessionMacos::mhopInputV4(const QByteArray& packet) {
  // Parse the IPv4 header
  auto header = reinterpret_cast<const struct ipv4header*>(packet.constData());
  quint16 hlen = (header->ihl & 0xF) * 4;
  if ((hlen < sizeof(struct ipv4header)) || (hlen > packet.length())) {
    return;
  }

  // Validate the header.
  if ((qFromBigEndian(header->source) != m_serverIpv4.toIPv4Address()) ||
      (qFromBigEndian(header->dest) != m_innerIpv4.toIPv4Address()) ||
      (header->proto != IPPROTO_UDP) || (header->ttl == 0) ||
      inetChecksum(header, sizeof(struct ipv4header)) != 0x0000) {
    return;
  }

  // Handle IPv4 defragmentation
  QByteArray dgram = packet.mid(hlen);
  if (header->frag & qToBigEndian<quint16>(0x3fff)) {
    dgram = mhopDefragV4(header, dgram);
    if (dgram.isEmpty()) {
      return;
    }
  }

  // Process the UDP header
  mhopInputUDP(m_serverIpv4, m_innerIpv4, dgram);
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

void WgSessionMacos::mhopInputV6(const QByteArray& packet) {
  // TODO: Implement Me!
}

void WgSessionMacos::mhopInputUDP(const QHostAddress& src,
                                  const QHostAddress& dst,
                                  const QByteArray& dgram) {
  const quint16* hdr = reinterpret_cast<const quint16*>(dgram.constData());
  if ((dgram.length() < 8) || (hdr[0] != htons(m_serverPort)) ||
      (hdr[1] != htons(m_innerPort)) || (htons(hdr[2]) > dgram.length())) {
    logger.debug() << "mhop drop udp:" << dgram.toHex();
    return;
  }

  QByteArray data = dgram.mid(8);
  if (hdr[3] != 0x0000) {
    // Validate the checksum
    quint16 cksum = udpChecksum(src, dst, htons(hdr[0]), htons(hdr[1]), data);
    if (hdr[3] != cksum) {
      logger.debug() << "mhop drop cksum:" << dgram.toHex();
      return;
    }
  }

  // At last - we can handle the payload.
  tunWrite(data, AF_INET);
}

void WgSessionMacos::setNetSocket(qintptr sd) {
  if (m_netSocket >= 0) {
    close(m_netSocket);
  }

  m_netSocket = sd;
  auto notifier = new QSocketNotifier(sd, QSocketNotifier::Read, this);
  connect(notifier, &QSocketNotifier::activated, this,
          &WgSessionMacos::netReadyRead);

  renegotiate();
}

void WgSessionMacos::setTunSocket(qintptr sd) {
  m_tunSocket = sd;
  auto notifier = new QSocketNotifier(sd, QSocketNotifier::Read, this);
  connect(notifier, &QSocketNotifier::activated, this,
          &WgSessionMacos::tunReadyRead);
}

void WgSessionMacos::setMtu(int mtu) {
  if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
    mtu -= WG_MTU_OVERHEAD;
  }
  if (mtu < IPV6_MMTU) {
    mtu = IPV6_MMTU;
  }
  m_tunmtu = mtu;

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

void WgSessionMacos::netReadyRead(QSocketDescriptor sd,
                                  QSocketNotifier::Type type) {
  QByteArray rxbuf;
  rxbuf.resize(m_tunmtu + WG_MTU_OVERHEAD);

  while (true) {
    // Try to read a packet from the network.
    int rxlen = recv(m_netSocket, (void*)rxbuf.data(), rxbuf.length(), MSG_DONTWAIT);
    if (rxlen < 0) {
      if (errno == EAGAIN) return;
      logger.debug() << "Recv error:" << strerror(errno);
      return;
    }
  
    if (m_config.m_hopType == InterfaceConfig::MultiHopExit) {
      mhopInput(rxbuf.first(rxlen));
    } else {
      netInput(rxbuf.first(rxlen));
    }
  }
}

void WgSessionMacos::tunReadyRead(QSocketDescriptor sd,
                                  QSocketNotifier::Type type) {
  // The tunnel socket is ready for reading.
  quint32 header = 0;
  QByteArray rxbuf;
  rxbuf.resize(m_tunmtu + 16);

  struct iovec iov[2];
  iov[0].iov_base = &header;
  iov[0].iov_len = sizeof(header);
  iov[1].iov_base = (void*)rxbuf.data();
  iov[1].iov_len = m_tunmtu;

  while (true) {
    // Try to read a packet from the tunnel.
    int len = readv(sd, iov, sizeof(iov) / sizeof(struct iovec));
    if (len < 0) {
      if (errno == EAGAIN) return;
      logger.debug() << "Tunnel error:" << strerror(errno);
      return;
    }
    int pktlen = len - sizeof(header);
    if ((pktlen < 0) || (pktlen > m_tunmtu)) {
      continue;
    }

    // I think there is a small bug in boringtun to do with message padding.
    // The wireguard protocol states that the encapsulated packet must first be
    // padded out to a multiple of 16 bytes in length, but boringtun does no
    // such padding during encryption. So let's do it manually ourself.
    int tail = pktlen % 16;
    if (tail) {
      int padsz = 16 - tail;
      memset(rxbuf.data() + pktlen, 0, padsz);
      pktlen += padsz;
    }

    // Encrypt the packet
    encrypt(rxbuf.first(pktlen));
  }
}

void WgSessionMacos::netWrite(const QByteArray& packet) {
  send(m_netSocket, packet.constData(), packet.length(), MSG_DONTWAIT);
}

void WgSessionMacos::tunWrite(const QByteArray& packet, quint32 family) {
  quint32 header = qToBigEndian<quint32>(family);
  struct iovec iov[2];
  iov[0].iov_base = &header;
  iov[0].iov_len = sizeof(header);
  iov[1].iov_base = (void*)packet.data();
  iov[1].iov_len = packet.length();
  writev(m_tunSocket, iov, 2);
}
