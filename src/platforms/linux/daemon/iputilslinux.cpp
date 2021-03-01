/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iputilslinux.h"
#include "daemon/wgutils.h"
#include "logger.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QHostAddress>

namespace {
Logger logger(LOG_LINUX, "IPUtilsLinux");
}

bool IPUtilsLinux::addInterfaceIPs(const InterfaceConfig& config) {
  // TODO - These are linux specific
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

bool IPUtilsLinux::addIP4AddressToDevice(const InterfaceConfig& config) {
  struct ifreq ifr;
  struct sockaddr_in* ifrAddr = (struct sockaddr_in*)&ifr.ifr_addr;

  // Name the interface and set family
  strncpy(ifr.ifr_name, WG_INTERFACE, IFNAMSIZ);
  ifr.ifr_addr.sa_family =
      AF_INET;  // TODO c++ question - why can't I use ifrAddr here?

  // Get the device address to add to interface
  QPair<QHostAddress, int> parsedAddr =
      QHostAddress::parseSubnet(config.m_deviceIpv4Address);
  QByteArray _deviceAddr = parsedAddr.first.toString().toLocal8Bit();
  char* deviceAddr = _deviceAddr.data();
  inet_pton(AF_INET, deviceAddr, &ifrAddr->sin_addr);

  // Create IPv4 socket to perform the ioctl operations on
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  int ret = ioctl(sockfd, SIOCSIFADDR, &ifr);
  if (ret) {
    logger.log() << "Failed to set IPv4: " << deviceAddr
                 << " -- Return code: " << ret;
    return false;
  }
  /*
   * TODO - Open question. We are not setting the netmask based on the /32
   * do we want to? Could there be other cidr values?
   */
  close(sockfd);

  // TODO c++ question - do I need to free any other objects?
  return true;
}

bool IPUtilsLinux::addIP6AddressToDevice(const InterfaceConfig& config) {
  // Set up the ifr and the companion ifr6
  struct in6_ifreq ifr6;
  ifr6.prefixlen = 64;

  // Get the device address to add to ifr6 interface
  QPair<QHostAddress, int> parsedAddr =
      QHostAddress::parseSubnet(config.m_deviceIpv6Address);
  QByteArray _deviceAddr = parsedAddr.first.toString().toLocal8Bit();
  char* deviceAddr = _deviceAddr.data();
  inet_pton(AF_INET6, deviceAddr, &ifr6.addr);

  // Create IPv6 socket to perform the ioctl operations on
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);

  // Get the index of named ifr and link with ifr6
  struct ifreq ifr;
  strncpy(ifr.ifr_name, WG_INTERFACE, IFNAMSIZ);
  ifr.ifr_addr.sa_family = AF_INET6;
  int ret = ioctl(sockfd, SIOGIFINDEX, &ifr);
  if (ret) {
    logger.log() << "Failed to get ifrindex. Return code: " << ret;
    return false;
  }
  ifr6.ifindex = ifr.ifr_ifindex;

  // Set ifr6 to the interface
  ret = ioctl(sockfd, SIOCSIFADDR, &ifr6);
  if (ret) {
    logger.log() << "Failed to set IPv6: " << deviceAddr
                 << " -- Return code: " << ret;
    return false;
  }

  close(sockfd);
  return true;
}
