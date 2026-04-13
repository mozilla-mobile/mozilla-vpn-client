/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macospingsender.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/errno.h>
#include <unistd.h>

#include <QSocketNotifier>
#include <QtEndian>

#include "leakdetector.h"
#include "logger.h"

namespace {

Logger logger("MacOSPingSender");

int identifier() { return (getpid() & 0xFFFF); }

};  // namespace

MacOSPingSender::MacOSPingSender(const QHostAddress& source, QObject* parent)
    : PingSender(parent) {
  MZ_COUNT_CTOR(MacOSPingSender);

  if (source.protocol() == QAbstractSocket::IPv6Protocol) {
    m_socket6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_ICMPV6);

    if (m_socket6 < 0) {
      logger.error() << "IPv6 socket creation failed";
      return;
    }

    struct sockaddr_in6 addr6;
    bzero(&addr6, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_len = sizeof(addr6);
    Q_IPV6ADDR qaddr = source.toIPv6Address();
    memcpy(&addr6.sin6_addr, &qaddr, sizeof(addr6.sin6_addr));

    if (bind(m_socket6, (struct sockaddr*)&addr6, sizeof(addr6)) != 0) {
      close(m_socket6);
      m_socket6 = -1;
      logger.error() << "IPv6 bind error:" << strerror(errno);
      return;
    }

    struct icmp6_filter filter;
    ICMP6_FILTER_SETBLOCKALL(&filter);
    ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filter);
    setsockopt(m_socket6, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
               sizeof(filter));

    m_notifier6 = new QSocketNotifier(m_socket6, QSocketNotifier::Read, this);
    connect(m_notifier6, &QSocketNotifier::activated, this,
            &MacOSPingSender::icmp6SocketReady);
    return;
  }

  if (getuid()) {
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  } else {
    m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  }
  if (m_socket < 0) {
    logger.error() << "Socket creation failed";
    return;
  }

  quint32 ipv4addr = INADDR_ANY;
  if (!source.isNull()) {
    ipv4addr = source.toIPv4Address();
  }
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_len = sizeof(addr);
  addr.sin_addr.s_addr = qToBigEndian<quint32>(ipv4addr);

  if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    close(m_socket);
    m_socket = -1;
    logger.error() << "bind error:" << strerror(errno);
    return;
  }

  m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this,
          &MacOSPingSender::socketReady);
}

MacOSPingSender::~MacOSPingSender() {
  MZ_COUNT_DTOR(MacOSPingSender);

  if (m_socket >= 0) {
    close(m_socket);
  }

  if (m_socket6 >= 0) {
    close(m_socket6);
  }
}

void MacOSPingSender::sendPing(const QHostAddress& dest, quint16 sequence) {
  if (dest.protocol() == QAbstractSocket::IPv6Protocol) {
    struct sockaddr_in6 addr6;
    bzero(&addr6, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_len = sizeof(addr6);
    Q_IPV6ADDR qaddr = dest.toIPv6Address();
    memcpy(&addr6.sin6_addr, &qaddr, sizeof(addr6.sin6_addr));

    struct icmp6_hdr packet;
    bzero(&packet, sizeof(packet));
    packet.icmp6_type = ICMP6_ECHO_REQUEST;
    packet.icmp6_id = identifier();
    packet.icmp6_seq = htons(sequence);

    if (sendto(m_socket6, &packet, sizeof(packet), MSG_NOSIGNAL,
               (struct sockaddr*)&addr6,
               sizeof(addr6)) != (ssize_t)sizeof(packet)) {
      logger.error() << "IPv6 ping sending failed:" << strerror(errno);
      emit criticalPingError();
    }

    return;
  }

  quint32 ipv4dest = dest.toIPv4Address();
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_len = sizeof(addr);
  addr.sin_addr.s_addr = qToBigEndian<quint32>(ipv4dest);

  struct icmp packet;
  bzero(&packet, sizeof packet);
  packet.icmp_type = ICMP_ECHO;
  packet.icmp_id = identifier();
  packet.icmp_seq = htons(sequence);
  packet.icmp_cksum = inetChecksum(&packet, sizeof(packet));

  if (sendto(m_socket, (char*)&packet, sizeof(packet), MSG_NOSIGNAL,
             (struct sockaddr*)&addr, sizeof(addr)) != sizeof(packet)) {
    logger.error() << "ping sending failed:" << strerror(errno);
    emit criticalPingError();
    return;
  }
}

void MacOSPingSender::socketReady() {
  struct msghdr msg;
  bzero(&msg, sizeof(msg));

  struct sockaddr_in addr;
  msg.msg_name = (caddr_t)&addr;
  msg.msg_namelen = sizeof(addr);

  struct iovec iov;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  u_char packet[IP_MAXPACKET];
  iov.iov_base = packet;
  iov.iov_len = IP_MAXPACKET;

  ssize_t rc = recvmsg(m_socket, &msg, MSG_DONTWAIT | MSG_NOSIGNAL);
  if (rc <= 0) {
    logger.error() << "Recvmsg failed:" << strerror(errno);
    return;
  }

  struct ip* ip = (struct ip*)packet;
  int hlen = ip->ip_hl << 2;
  struct icmp* icmp = (struct icmp*)(((char*)packet) + hlen);

  if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == identifier()) {
    emit recvPing(htons(icmp->icmp_seq));
  }
}

void MacOSPingSender::icmp6SocketReady() {
  u_char packet[IP_MAXPACKET];
 
  ssize_t rc =
      recv(m_socket6, packet, sizeof(packet), MSG_DONTWAIT | MSG_NOSIGNAL);
 
  if (rc < (ssize_t)sizeof(struct icmp6_hdr)) {
    if (rc < 0) {
      logger.error() << "ICMPv6 recv failed:" << strerror(errno);
    }
 
    return;
  }

  struct icmp6_hdr icmp6;
  memcpy(&icmp6, packet, sizeof(icmp6));

  if (icmp6.icmp6_type == ICMP6_ECHO_REPLY && icmp6.icmp6_id == identifier()) {
    emit recvPing(htons(icmp6.icmp6_seq));
  }
}
