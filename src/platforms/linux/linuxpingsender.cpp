/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxpingsender.h"
#include "leakdetector.h"
#include "logger.h"

#include <QSocketNotifier>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
Logger logger({LOG_LINUX, LOG_NETWORKING}, "LinuxPingSender");
}

LinuxPingSender::LinuxPingSender(const QString& source, QObject* parent)
    : PingSender(parent) {
  MVPN_COUNT_CTOR(LinuxPingSender);
  logger.log() << "LinuxPingSender(" + source + ") created";

  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (m_socket < 0) {
    logger.log() << "Socket creation error: " << strerror(errno);
    return;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  if (inet_aton(source.toLocal8Bit().constData(), &addr.sin_addr) == 0) {
    logger.log() << "source" << source << "error:" << strerror(errno);
    return;
  }
  if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    logger.log() << "bind error:" << strerror(errno);
    return;
  }

  m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this,
          &LinuxPingSender::socketReady);
}

LinuxPingSender::~LinuxPingSender() {
  MVPN_COUNT_DTOR(LinuxPingSender);
  if (m_socket >= 0) {
    close(m_socket);
  }
}

void LinuxPingSender::sendPing(const QString& dest, quint16 sequence) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  if (inet_aton(dest.toLocal8Bit().constData(), &addr.sin_addr) == 0) {
    return;
  }

  struct icmphdr packet;
  memset(&packet, 0, sizeof(packet));
  packet.type = ICMP_ECHO;
  packet.un.echo.sequence = htons(sequence);

  int rc = sendto(m_socket, &packet, sizeof(packet), 0, (struct sockaddr*)&addr,
                  sizeof(addr));
  if (rc < 0) {
    logger.log() << "failed to send:" << strerror(errno);
  }
}

void LinuxPingSender::socketReady() {
  socklen_t slen = 0;
  unsigned char data[2048];
  int rc = recvfrom(m_socket, data, sizeof data, MSG_DONTWAIT, NULL, &slen);
  if (rc <= 0) {
    logger.log() << "recvfrom failed:" << strerror(errno);
    return;
  }

  struct icmphdr packet;
  if (rc >= (int)sizeof(packet)) {
    memcpy(&packet, data, sizeof(packet));
    if (packet.type == ICMP_ECHOREPLY) {
      emit recvPing(htons(packet.un.echo.sequence));
    }
  }
}
