/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxpingsendworker.h"
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
Logger logger({LOG_LINUX, LOG_NETWORKING}, "LinuxPingSendWorker");
}

LinuxPingSendWorker::LinuxPingSendWorker() {
  MVPN_COUNT_CTOR(LinuxPingSendWorker);
}

LinuxPingSendWorker::~LinuxPingSendWorker() {
  MVPN_COUNT_DTOR(LinuxPingSendWorker);
}

void LinuxPingSendWorker::sendPing(const QString& destination,
                                   const QString& source) {
  logger.log() << "LinuxPingSendWorker - start" << destination << "from"
               << source;

  struct in_addr dst;
  struct in_addr src;
  struct sockaddr_in addr;

  if (inet_aton(destination.toLocal8Bit().constData(), &dst) == 0) {
    logger.log() << "Lookup error";
    emit pingFailed();
    return;
  }
  if (inet_aton(source.toLocal8Bit().constData(), &src) == 0) {
    logger.log() << "source address error";
    emit pingFailed();
    return;
  }

  Q_ASSERT(m_socket == 0);
  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (m_socket < 0) {
    logger.log() << "Socket creation error";
    emit pingFailed();
    releaseObjects();
    return;
  }

  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr = src;
  if (bind(m_socket, (struct sockaddr*)&addr, sizeof addr) != 0) {
    logger.log() << "bind error";
    emit pingFailed();
    releaseObjects();
    return;
  }

  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr = dst;

  struct icmphdr packet;
  memset(&packet, 0, sizeof packet);
  packet.type = ICMP_ECHO;

  int rc = sendto(m_socket, &packet, sizeof packet, 0, (struct sockaddr*)&addr,
                  sizeof addr);
  if (rc <= 0) {
    logger.log() << "Sending ping failed";
    emit pingFailed();
    releaseObjects();
    return;
  }

  logger.log() << "Ping sent";

  m_socketNotifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);
  connect(m_socketNotifier, &QSocketNotifier::activated,
          [this](
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
              QSocketDescriptor socket, QSocketNotifier::Type
#else
          int socket
#endif
          ) {
            socklen_t slen = 0;
            unsigned char data[2048];
            int rc = recvfrom(socket, data, sizeof data, 0, NULL, &slen);
            if (rc <= 0) {
              logger.log() << "Recvfrom failed";
              emit pingFailed();
              releaseObjects();
              return;
            }

            logger.log() << "Ping reply received";
            emit pingSucceeded();
            releaseObjects();
          });
}

void LinuxPingSendWorker::releaseObjects() {
  if (m_socket > 0) {
    close(m_socket);
    m_socket = 0;
  }
  m_socket = 0;

  if (m_socketNotifier) {
    delete m_socketNotifier;
    m_socketNotifier = nullptr;
  }
}
