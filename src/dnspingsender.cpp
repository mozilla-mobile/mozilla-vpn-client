/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnspingsender.h"
#include "leakdetector.h"
#include "logger.h"

#include <QNetworkDatagram>
#include <QtEndian>

#include <string.h>

// A quick and dirty DNS Header structure definition from RFC1035,
// Section 4.1.1: Header Section format.
struct dnsHeader {
  quint16 id;
  quint16 flags;
  quint16 qdcount;
  quint16 ancount;
  quint16 nscount;
  quint16 arcount;
};

// Bit definitions for the DNS flags field.
constexpr quint16 DNS_FLAG_QR = 0x8000;
constexpr quint16 DNS_FLAG_OPCODE = 0x7800;
constexpr quint16 DNS_FLAG_OPCODE_QUERY = (0 << 11);
constexpr quint16 DNS_FLAG_OPCODE_IQUERY = (1 << 11);
constexpr quint16 DNS_FLAG_OPCODE_STATUS = (2 << 11);
constexpr quint16 DNS_FLAG_AA = 0x0400;
constexpr quint16 DNS_FLAG_TC = 0x0200;
constexpr quint16 DNS_FLAG_RD = 0x0100;
constexpr quint16 DNS_FLAG_RA = 0x0080;
constexpr quint16 DNS_FLAG_Z = 0x0070;
constexpr quint16 DNS_FLAG_RCODE = 0x000F;
constexpr quint16 DNS_FLAG_RCODE_NO_ERROR = (0 << 0);
constexpr quint16 DNS_FLAG_RCODE_FORMAT_ERROR = (1 << 0);
constexpr quint16 DNS_FLAG_RCODE_SERVER_FAILURE = (2 << 0);
constexpr quint16 DNS_FLAG_RCODE_NAME_ERROR = (3 << 0);
constexpr quint16 DNS_FLAG_RCODE_NOT_IMPLEMENTED = (4 << 0);
constexpr quint16 DNS_FLAG_RCODE_REFUSED = (5 << 0);

constexpr quint16 DNS_PORT = 53;

namespace {
Logger logger(LOG_NETWORKING, "DnsPingSender");
}

DnsPingSender::DnsPingSender(const QString& source, QObject* parent)
    : PingSender(parent) {
  MVPN_COUNT_CTOR(DnsPingSender);

  if (source.isEmpty()) {
    m_socket.bind();
  } else {
    m_socket.bind(QHostAddress(source));
  }

  connect(&m_socket, &QUdpSocket::readyRead, this, &DnsPingSender::readData);
}

DnsPingSender::~DnsPingSender() { MVPN_COUNT_DTOR(DnsPingSender); }

void DnsPingSender::sendPing(const QString& dest, quint16 sequence) {
  // Assemble an empty DNS status query.
  struct dnsHeader packet;
  memset(&packet, 0, sizeof(packet));
  packet.id = qToBigEndian<quint16>(sequence);
  packet.flags = qToBigEndian<quint16>(DNS_FLAG_OPCODE_STATUS);
  packet.qdcount = 0;
  packet.ancount = 0;
  packet.nscount = 0;
  packet.arcount = 0;

  // Send the datagram.
  m_socket.writeDatagram(reinterpret_cast<char*>(&packet), sizeof(packet),
                         QHostAddress(dest), DNS_PORT);
}

void DnsPingSender::readData() {
  while (m_socket.hasPendingDatagrams()) {
    QNetworkDatagram reply = m_socket.receiveDatagram();
    if (!reply.isValid()) {
      break;
    }

    // Extract the header from the DNS response.
    QByteArray payload = reply.data();
    struct dnsHeader header;
    if (payload.length() < static_cast<int>(sizeof(header))) {
      logger.debug() << "Received bogus DNS reply: truncated header";
      continue;
    }
    memcpy(&header, payload.constData(), sizeof(header));

    // Perfom some checks to ensure this is the reply we were expecting.
    quint16 flags = qFromBigEndian<quint16>(header.flags);
    if ((flags & DNS_FLAG_QR) == 0) {
      logger.debug() << "Received bogus DNS reply: QR == query";
      continue;
    }
    if ((flags & DNS_FLAG_OPCODE) != DNS_FLAG_OPCODE_STATUS) {
      logger.debug() << "Received bogus DNS reply: OPCODE != status";
      continue;
    }

    emit recvPing(qFromBigEndian<quint16>(header.id));
  }
}
