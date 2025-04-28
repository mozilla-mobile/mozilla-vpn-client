/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionmacos.h"

#include <netinet/in.h>

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

constexpr const int WG_SESSION_TICK_INTERVAL = 1000;
constexpr const int WG_DEFRAG_TIMEOUT = 3000;
constexpr const int WG_PACKET_OVERHEAD = 32;

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
}

WgSessionMacos::~WgSessionMacos() {
  MZ_COUNT_DTOR(WgSessionMacos);
  logger.debug() << "WgSessionMacos destroyed.";
  if (m_tunnel) {
    tunnel_free(m_tunnel);
  }
}

void WgSessionMacos::connectNotify(const QMetaMethod& signal) {
  if (signal.name() == "mhopOutput") {
    m_mhopEnabled = true;
  }
}

void WgSessionMacos::processResult(int op, const QByteArray& buf) {
  switch (op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      emit netOutput(buf);
      // Encapsulate and emit for multihop, only if someone is listening.
      if (m_mhopEnabled) {
        emit mhopOutput(mhopEncapsulate(buf));
      }
      break;

    case WIREGUARD_ERROR:
      logger.warning() << "Wireguard error";
      break;

    case WRITE_TO_TUNNEL_IPV4:
      [[fallthrough]];
    case WRITE_TO_TUNNEL_IPV6:
      emit decrypted(buf);
      break;
  }
}

void WgSessionMacos::timeout() {
  QByteArray buffer;
  buffer.resize(1500);

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
  QByteArray buffer;
  buffer.resize(1500);

  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_force_handshake(m_tunnel, bufptr, buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::encrypt(const QByteArray& data) {
  QByteArray buffer;
  buffer.resize(data.length() + WG_PACKET_OVERHEAD);

  const uint8_t* dataptr = reinterpret_cast<const uint8_t*>(data.constData());
  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result =
      wireguard_write(m_tunnel, dataptr, data.length(), bufptr, buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::netInput(const QByteArray& data) {
  QByteArray buffer;
  buffer.resize(data.length());

  const uint8_t* dataptr = reinterpret_cast<const uint8_t*>(data.constData());
  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_read(m_tunnel, dataptr, data.length(), bufptr,
                               buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::readyRead() {
  QUdpSocket* sock = qobject_cast<QUdpSocket*>(QObject::sender());
  if (!sock) {
    return;
  }

  while (true) {
    QNetworkDatagram dgram = sock->receiveDatagram();
    if (!dgram.isValid()) {
      return;
    }
    netInput(dgram.data());
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

void WgSessionMacos::mhopInput(const QByteArray& packet) {
  quint8 version = (packet[0] >> 4);
  if (version == 4) {
    mhopInputV4(packet);
  } else if (version == 6) {
    mhopInputV6(packet);
  }
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
  netInput(data);
}
