/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosroutemonitor.h"
#include "leakdetector.h"
#include "logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
Logger logger(LOG_MACOS, "MacosRouteMonitor");

template <typename T>
static T* sockaddr_cast(QByteArray& data) {
  const struct sockaddr* sa = (const struct sockaddr*)data.constData();
  Q_ASSERT(sa->sa_len <= data.length());
  if (data.length() >= (int)sizeof(T)) {
    return (T*)data.data();
  }
  return nullptr;
}

}  // namespace

MacosRouteMonitor::MacosRouteMonitor(const QString& ifname, QObject* parent)
    : QObject(parent), m_ifname(ifname) {
  MVPN_COUNT_CTOR(MacosRouteMonitor);
  logger.log() << "MacosRouteMonitor created.";

  m_rtsock = socket(PF_ROUTE, SOCK_RAW, 0);
  if (m_rtsock < 0) {
    logger.log() << "Failed to create routing socket:" << strerror(errno);
  }

  m_notifier = new QSocketNotifier(m_rtsock, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this,
          &MacosRouteMonitor::rtsockReady);
}

MacosRouteMonitor::~MacosRouteMonitor() {
  MVPN_COUNT_DTOR(MacosRouteMonitor);
  if (m_rtsock >= 0) {
    close(m_rtsock);
  }
  logger.log() << "MacosRouteMonitor created.";
}

void MacosRouteMonitor::handleRtmAdd(const struct rt_msghdr* rtm,
                                     const QByteArray& payload) {
  QList<QByteArray> addrlist = parseAddrList(payload);

  // Ignore routing changes on the tunnel interfaces
  if ((rtm->rtm_addrs & RTA_DST) && (rtm->rtm_addrs & RTA_GATEWAY)) {
    if (m_ifname == addrToString(addrlist[1])) {
      return;
    }
  }

  QStringList list;
  for (auto addr : addrlist) {
    list.append(addrToString(addr));
  }
  logger.log() << "Route added by" << rtm->rtm_pid
               << QString("addrs(%1):").arg(rtm->rtm_addrs) << list.join(" ");
}

void MacosRouteMonitor::handleRtmDelete(const struct rt_msghdr* rtm,
                                        const QByteArray& payload) {
  QList<QByteArray> addrlist = parseAddrList(payload);

  // Ignore routing changes on the tunnel interfaces
  if ((rtm->rtm_addrs & RTA_DST) && (rtm->rtm_addrs & RTA_GATEWAY)) {
    if (m_ifname == addrToString(addrlist[1])) {
      return;
    }
  }

  QStringList list;
  for (auto addr : addrlist) {
    list.append(addrToString(addr));
  }
  logger.log() << "Route deleted by" << rtm->rtm_pid
               << QString("addrs(%1):").arg(rtm->rtm_addrs) << list.join(" ");
}

void MacosRouteMonitor::handleRtmChange(const struct rt_msghdr* rtm,
                                        const QByteArray& payload) {
  QList<QByteArray> addrlist = parseAddrList(payload);

  // Ignore routing changes on the tunnel interfaces
  if ((rtm->rtm_addrs & RTA_DST) && (rtm->rtm_addrs & RTA_GATEWAY)) {
    if (m_ifname == addrToString(addrlist[1])) {
      return;
    }
  }

  QStringList list;
  for (auto addr : addrlist) {
    list.append(addrToString(addr));
  }
  logger.log() << "Route chagned by" << rtm->rtm_pid
               << QString("addrs(%1):").arg(rtm->rtm_addrs) << list.join(" ");
}

void MacosRouteMonitor::rtsockReady() {
  char buf[1024];
  ssize_t len = recv(m_rtsock, buf, sizeof(buf), MSG_DONTWAIT);
  if (len <= 0) {
    return;
  }

#ifndef RTMSG_NEXT
#  define RTMSG_NEXT(_rtm_) \
    (struct rt_msghdr*)((char*)(_rtm_) + (_rtm_)->rtm_msglen)
#endif

  struct rt_msghdr* end = (struct rt_msghdr*)(&buf[len]);
  for (struct rt_msghdr* rtm = (struct rt_msghdr*)buf; rtm < end;
       rtm = RTMSG_NEXT(rtm)) {
    // Handle the routing message.
    QByteArray message((char*)rtm, rtm->rtm_msglen);
    switch (rtm->rtm_type) {
      case RTM_ADD:
        message.remove(0, sizeof(struct rt_msghdr));
        handleRtmAdd(rtm, message);
        break;
      case RTM_DELETE:
        message.remove(0, sizeof(struct rt_msghdr));
        handleRtmDelete(rtm, message);
        break;
      case RTM_CHANGE:
        message.remove(0, sizeof(struct rt_msghdr));
        handleRtmChange(rtm, message);
        break;
      default:
        logger.log() << "Unknown routing message:" << rtm->rtm_type;
        break;
    }
  }
}

// dummy implementations for now
bool MacosRouteMonitor::insertRoute(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}

bool MacosRouteMonitor::deleteRoute(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}

// static
QList<QByteArray> MacosRouteMonitor::parseAddrList(const QByteArray& payload) {
  QList<QByteArray> list;
  int offset = 0;
  constexpr int minlen = offsetof(struct sockaddr, sa_len) + sizeof(u_short);

  while ((offset + minlen) <= payload.length()) {
    struct sockaddr* sa = (struct sockaddr*)(payload.constData() + offset);
    int paddedSize = sa->sa_len;
    if (!paddedSize || (paddedSize % sizeof(uint32_t))) {
      paddedSize += sizeof(uint32_t) - (paddedSize % sizeof(uint32_t));
    }
    if ((offset + paddedSize) > payload.length()) {
      break;
    }
    list.append(payload.mid(offset, paddedSize));
    offset += paddedSize;
  }
  return list;
}

// static
QString MacosRouteMonitor::addrToString(const struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) {
    const struct sockaddr_in* sin = (const struct sockaddr_in*)sa;
    return QString(inet_ntoa(sin->sin_addr));
  }
  if (sa->sa_family == AF_INET6) {
    const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)sa;
    char buf[INET6_ADDRSTRLEN];
    return QString(inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof(buf)));
  }
  if (sa->sa_family == AF_LINK) {
    const struct sockaddr_dl* sdl = (const struct sockaddr_dl*)sa;
    return QString(link_ntoa(sdl));
  }
  return QString("unknown(af=%1)").arg(sa->sa_family);
}

// static
QString MacosRouteMonitor::addrToString(const QByteArray& data) {
  const struct sockaddr* sa = (const struct sockaddr*)data.constData();
  Q_ASSERT(sa->sa_len <= data.length());
  return addrToString(sa);
}
