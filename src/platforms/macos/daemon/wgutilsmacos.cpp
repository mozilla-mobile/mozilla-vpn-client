/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgutilsmacos.h"

#include <errno.h>
#include <fcntl.h>
#include <net/if_utun.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/un.h>
#include <unistd.h>

#include <QUdpSocket>
#include <QVariant>
#include <QtEndian>

#include "interfaceconfig.h"
#include "leakdetector.h"
#include "logger.h"
#include "wgsessionmacos.h"
#include "wireguard_ffi.h"

namespace {
Logger logger("WgUtilsMacos");
};  // namespace

WgUtilsMacos::WgUtilsMacos(QObject* parent) : WireguardUtils(parent) {
  MZ_COUNT_CTOR(WgUtilsMacos);
  logger.debug() << "WgUtilsMacos created.";
}

WgUtilsMacos::~WgUtilsMacos() {
  MZ_COUNT_DTOR(WgUtilsMacos);
  logger.debug() << "WgUtilsMacos destroyed.";
}

bool WgUtilsMacos::addInterface(const InterfaceConfig& config) {
  Q_UNUSED(config);
  if (m_tunfd > 0) {
    logger.warning() << "Unable to start: tunnel already running";
    return false;
  }

  // Create the userspace tunnel device.
  int tunfd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
  if (tunfd < 0) {
    logger.warning() << "socket creation failed:" << strerror(errno);
    return false;
  }
  auto guard = qScopeGuard([tunfd]() { close(tunfd); });

  // Connect to the utun control kernel service.
  struct ctl_info info = {.ctl_name = "com.apple.net.utun_control"};
  int err = ioctl(tunfd, CTLIOCGINFO, &info);
  if (err < 0) {
    logger.warning() << "kernel utun lookup failed:" << strerror(errno);
    return false;
  }
  struct sockaddr_ctl addr = {};
  addr.sc_len = sizeof(addr);
  addr.sc_family = AF_SYSTEM;
  addr.ss_sysaddr = AF_SYS_CONTROL;
  addr.sc_id = info.ctl_id;
  addr.sc_unit = 0;
  err =
      ::connect(tunfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  if (err < 0) {
    logger.warning() << "kernel utun connect failed:" << strerror(errno);
    return false;
  }

  // Set the tunnel to non-blocking mode.
  fcntl(tunfd, F_SETFL, fcntl(tunfd, F_GETFL) | O_NONBLOCK);

  // Get the tunnel device's name.
  struct ifreq ifr;
  socklen_t ifnamesize = sizeof(ifr.ifr_name);
  err = getsockopt(tunfd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifr.ifr_name,
                   &ifnamesize);
  if (err < 0) {
    logger.warning() << "utun name loookup failed:" << strerror(errno);
    return false;
  }

  // Set a base MTU, it will get updated later.
  ifr.ifr_mtu = IPV6_MMTU;
  if (ioctl(tunfd, SIOCSIFMTU, &ifr) != 0) {
    logger.error() << "Failed to set MTU:" << strerror(errno);
    return false;
  }

  // Bring the device up.
  err = ioctl(tunfd, SIOCGIFFLAGS, &ifr);
  if (err != 0) {
    logger.error() << "Failed to get interface flags:" << strerror(errno);
    return false;
  }
  ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  err = ioctl(tunfd, SIOCSIFFLAGS, &ifr);
  if (err != 0) {
    logger.error() << "Failed to set device up:" << strerror(errno);
    return false;
  }

  // Create a socketpair to bridge between multihop connections.
  int sv[2];
  err = socketpair(AF_UNIX, SOCK_DGRAM, 0, sv);
  if (err != 0) {
    logger.error() << "Failed to create multihop bridging sockets:" << strerror(errno);
    return false;
  }
  m_mhopEntrySocket = sv[0];
  m_mhopExitSocket = sv[1];

  // The interface was successfully created.
  logger.info() << "Created tunnel interface:" << ifr.ifr_name;
  m_ifname = QString::fromUtf8(ifr.ifr_name);
  m_tunfd = tunfd;
  m_rtmonitor = new MacosRouteMonitor(m_ifname, this);
  connect(m_rtmonitor, &MacosRouteMonitor::defaultRouteUpdated, this,
          &WgUtilsMacos::mtuUpdate);

  // We can dismiss the cleanup guard.
  guard.dismiss();
  return true;
}

bool WgUtilsMacos::deleteInterface() {
  if (m_tunfd >= 0) {
    close(m_tunfd);
    m_ifname.clear();
    m_tunfd = -1;
  }
  if (m_mhopEntrySocket >= 0) {
    close(m_mhopEntrySocket);
    m_mhopEntrySocket = -1;
  }
  if (m_mhopExitSocket >= 0) {
    close(m_mhopExitSocket);
    m_mhopExitSocket = -1;
  }

  if (m_rtmonitor) {
    delete m_rtmonitor;
    m_rtmonitor = nullptr;
  }

  return true;
}

bool WgUtilsMacos::updatePeer(const InterfaceConfig& config) {
  // Destroy the old peer if it exists and create the new one.
  WgSessionMacos* peer = m_peers.take(config.m_serverPublicKey);
  if (peer) {
    delete peer;
  }
  peer = new WgSessionMacos(config, this);

  // Create a socket to handle outbound packet flows.
  if (config.m_hopType != InterfaceConfig::MultiHopExit) {
    struct sockaddr_in sin;
    quint32 dst = QHostAddress(config.m_serverIpv4AddrIn).toIPv4Address();
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(sin);
    sin.sin_port = qToBigEndian<quint16>(config.m_serverPort);
    sin.sin_addr.s_addr = qToBigEndian<quint32>(dst);
    if (!peer->start((struct sockaddr*)&sin, sizeof(sin))) {
      return false;
    }

    if (m_rtmonitor != nullptr) {
      m_rtmonitor->addExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
      m_rtmonitor->addExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
    }
  } else {
    peer->start(dup(m_mhopExitSocket));
  }

  // Single-hop and multihop exit peers send and receive packets from the tunnel
  if (config.m_hopType != InterfaceConfig::MultiHopEntry) {
    peer->setTunSocket(m_tunfd);
  } else {
    peer->setTunSocket(m_mhopEntrySocket);
  }

  m_peers.insert(config.m_serverPublicKey, peer);
  return true;
}

bool WgUtilsMacos::deletePeer(const InterfaceConfig& config) {
  // Destroy the old peer, if it exists.
  WgSessionMacos* peer = m_peers.take(config.m_serverPublicKey);
  if (peer) {
    delete peer;
  }

  // Clear exclustion routes for this peer.
  if (m_rtmonitor && (config.m_hopType != InterfaceConfig::MultiHopExit)) {
    m_rtmonitor->deleteExclusionRoute(IPAddress(config.m_serverIpv4AddrIn));
    m_rtmonitor->deleteExclusionRoute(IPAddress(config.m_serverIpv6AddrIn));
  }

  return true;
}

QList<WireguardUtils::PeerStatus> WgUtilsMacos::getPeerStatus() {
  QList<PeerStatus> peerList;
  for (auto i = m_peers.constBegin(); i != m_peers.constEnd(); i++) {
    peerList.append(i.value()->status());
  }
  return peerList;
}

bool WgUtilsMacos::updateRoutePrefix(const IPAddress& prefix) {
  if (!m_rtmonitor) {
    return false;
  }
  if (prefix.prefixLength() > 0) {
    return m_rtmonitor->insertRoute(prefix);
  }

  // Ensure that we do not replace the default route.
  if (prefix.type() == QAbstractSocket::IPv4Protocol) {
    return m_rtmonitor->insertRoute(IPAddress("0.0.0.0/1")) &&
           m_rtmonitor->insertRoute(IPAddress("128.0.0.0/1"));
  }
  if (prefix.type() == QAbstractSocket::IPv6Protocol) {
    return m_rtmonitor->insertRoute(IPAddress("::/1")) &&
           m_rtmonitor->insertRoute(IPAddress("8000::/1"));
  }

  return false;
}

bool WgUtilsMacos::deleteRoutePrefix(const IPAddress& prefix) {
  if (!m_rtmonitor) {
    return false;
  }

  if (prefix.prefixLength() > 0) {
    return m_rtmonitor->deleteRoute(prefix);
  }
  // Ensure that we do not replace the default route.
  if (prefix.type() == QAbstractSocket::IPv4Protocol) {
    return m_rtmonitor->deleteRoute(IPAddress("0.0.0.0/1")) &&
           m_rtmonitor->deleteRoute(IPAddress("128.0.0.0/1"));
  } else if (prefix.type() == QAbstractSocket::IPv6Protocol) {
    return m_rtmonitor->deleteRoute(IPAddress("::/1")) &&
           m_rtmonitor->deleteRoute(IPAddress("8000::/1"));
  } else {
    return false;
  }
}

bool WgUtilsMacos::excludeLocalNetworks(const QList<IPAddress>& routes) {
  if (!m_rtmonitor) {
    return false;
  }

  // Explicitly discard LAN traffic that makes its way into the tunnel. This
  // doesn't really exclude the LAN traffic, we just don't take any action to
  // overrule the routes of other interfaces.
  bool result = true;
  for (const auto& prefix : routes) {
    logger.error() << "Attempting to exclude:" << prefix.toString();
    if (!m_rtmonitor->insertRoute(prefix, RTF_IFSCOPE | RTF_REJECT)) {
      result = false;
    }
  }

  // TODO: A kill switch would be nice though :)
  return result;
}

void WgUtilsMacos::mtuUpdate(int proto, const QHostAddress& gateway,
                             int ifindex, int mtu) {
  // TODO: Support IPv6 servers.
  if (proto != QAbstractSocket::IPv4Protocol) {
    return;
  }
  if (gateway.isNull()) {
    return;
  }

  // Update the tunnel device's MTU
  struct ifreq ifr;
  ifr.ifr_mtu = mtu - WgSessionMacos::WG_MTU_OVERHEAD;
  if (ifr.ifr_mtu < IPV6_MMTU) {
    ifr.ifr_mtu = IPV6_MMTU;
  }
  logger.info() << "Updating MTU to" << ifr.ifr_mtu;

  for (WgSessionMacos* peer : m_peers) {
    peer->setMtu(ifr.ifr_mtu);
  }
}
