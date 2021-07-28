/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iputilsmacos.h"
#include "leakdetector.h"
#include "logger.h"
#include "macosdaemon.h"
#include "daemon/wireguardutils.h"

#include <QHostAddress>
#include <QScopeGuard>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_var.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <sys/ioctl.h>
#include <unistd.h>

constexpr uint32_t ETH_MTU = 1500;
constexpr uint32_t WG_MTU_OVERHEAD = 80;

namespace {
Logger logger(LOG_MACOS, "IPUtilsMacos");
}

IPUtilsMacos::IPUtilsMacos(QObject* parent) : IPUtils(parent) {
  MVPN_COUNT_CTOR(IPUtilsMacos);
  logger.log() << "IPUtilsMacos created.";
}

IPUtilsMacos::~IPUtilsMacos() {
  MVPN_COUNT_DTOR(IPUtilsMacos);
  logger.log() << "IPUtilsMacos destroyed.";
}

bool IPUtilsMacos::addInterfaceIPs(const InterfaceConfig& config) {
  if (!addIP4AddressToDevice(config)) {
    return false;
  }
  if (config.m_ipv6Enabled) {
    if (!addIP6AddressToDevice(config)) {
      return false;
    }
  }
  return true;
}

bool IPUtilsMacos::setMTUAndUp(const InterfaceConfig& config) {
  Q_UNUSED(config);
  QString ifname = MacOSDaemon::instance()->wgutils()->interfaceName();

  // Create socket file descriptor to perform the ioctl operations on
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd < 0) {
    logger.log() << "Failed to create ioctl socket.";
    return false;
  }
  auto guard = qScopeGuard([&] { close(sockfd); });

  // Setup the interface to interact with
  struct ifreq ifr;
  strncpy(ifr.ifr_name, qPrintable(ifname), IFNAMSIZ);

  // MTU
  // FIXME: We need to know how many layers deep this particular
  // interface is into a tunnel to work effectively. Otherwise
  // we will run into fragmentation issues.
  ifr.ifr_mtu = ETH_MTU - WG_MTU_OVERHEAD;
  int ret = ioctl(sockfd, SIOCSIFMTU, &ifr);
  if (ret) {
    logger.log() << "Failed to set MTU -- Return code: " << ret;
    return false;
  }

  // Up
  ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
  if (ret) {
    logger.log() << "Failed to set device up -- Return code: " << ret;
    return false;
  }

  return true;
}

bool IPUtilsMacos::addIP4AddressToDevice(const InterfaceConfig& config) {
  Q_UNUSED(config);
  QString ifname = MacOSDaemon::instance()->wgutils()->interfaceName();
  struct ifreq ifr;
  struct sockaddr_in* ifrAddr = (struct sockaddr_in*)&ifr.ifr_addr;

  // Name the interface and set family
  strncpy(ifr.ifr_name, qPrintable(ifname), IFNAMSIZ);
  ifr.ifr_addr.sa_family = AF_INET;

  // Get the device address to add to interface
  QPair<QHostAddress, int> parsedAddr =
      QHostAddress::parseSubnet(config.m_deviceIpv4Address);
  QByteArray _deviceAddr = parsedAddr.first.toString().toLocal8Bit();
  char* deviceAddr = _deviceAddr.data();
  inet_pton(AF_INET, deviceAddr, &ifrAddr->sin_addr);

  // Create IPv4 socket to perform the ioctl operations on
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd < 0) {
    logger.log() << "Failed to create ioctl socket.";
    return false;
  }
  auto guard = qScopeGuard([&] { close(sockfd); });

  // Set ifr to interface
  int ret = ioctl(sockfd, SIOCSIFADDR, &ifr);
  if (ret) {
    logger.log() << "Failed to set IPv4: " << deviceAddr
                 << "error:" << strerror(errno);
    return false;
  }
  return true;
}

bool IPUtilsMacos::addIP6AddressToDevice(const InterfaceConfig& config) {
  Q_UNUSED(config);
  QString ifname = MacOSDaemon::instance()->wgutils()->interfaceName();
  struct in6_aliasreq ifr6;

  // Name the interface and set family
  strncpy(ifr6.ifra_name, qPrintable(ifname), IFNAMSIZ);
  ifr6.ifra_addr.sin6_family = AF_INET6;
  ifr6.ifra_addr.sin6_len = sizeof(ifr6.ifra_addr);
  ifr6.ifra_lifetime.ia6t_vltime = ifr6.ifra_lifetime.ia6t_pltime = 0xffffffff;
  ifr6.ifra_prefixmask.sin6_family = AF_INET6;
  ifr6.ifra_prefixmask.sin6_len = sizeof(ifr6.ifra_prefixmask);
  memset(&ifr6.ifra_prefixmask.sin6_addr, 0xff, 8); /* 8 * 8 = 64 bit */

  // Get the device address to add to interface
  QPair<QHostAddress, int> parsedAddr =
      QHostAddress::parseSubnet(config.m_deviceIpv6Address);
  QByteArray _deviceAddr = parsedAddr.first.toString().toLocal8Bit();
  char* deviceAddr = _deviceAddr.data();
  inet_pton(AF_INET6, deviceAddr, &ifr6.ifra_addr.sin6_addr);

  // Create IPv4 socket to perform the ioctl operations on
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd < 0) {
    logger.log() << "Failed to create ioctl socket.";
    return false;
  }
  auto guard = qScopeGuard([&] { close(sockfd); });

  // Set ifr to interface
  int ret = ioctl(sockfd, SIOCAIFADDR_IN6, &ifr6);
  if (ret) {
    logger.log() << "Failed to set IPv6: " << deviceAddr
                 << "error:" << strerror(errno);
    return false;
  }
  return true;
}
