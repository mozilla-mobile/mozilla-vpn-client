/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilslinux.h"
#include "leakdetector.h"
#include "logger.h"

#include <QScopeGuard>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// Import wireguard C library for Linux
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#if defined(__cplusplus)
}
#endif
// End import wireguard

namespace {
Logger logger(LOG_LINUX, "WireguardUtilsLinux");
}

WireguardUtilsLinux::WireguardUtilsLinux(QObject* parent)
    : WireguardUtils(parent) {
  MVPN_COUNT_CTOR(WireguardUtilsLinux);
  logger.log() << "WireguardUtilsLinux created.";
}

WireguardUtilsLinux::~WireguardUtilsLinux() {
  MVPN_COUNT_DTOR(WireguardUtilsLinux);
  logger.log() << "WireguardUtilsLinux destroyed.";
}

bool WireguardUtilsLinux::interfaceExists() {
  // As currentInterfaces only gets wireguard interfaces, this method
  // also confirms an interface as being a wireguard interface.
  return currentInterfaces().contains(WG_INTERFACE);
};

bool WireguardUtilsLinux::addInterface() {
  int returnCode = wg_add_device(WG_INTERFACE);
  if (returnCode != 0) {
    qWarning("Adding interface `%s` failed with return code: %d", WG_INTERFACE,
             returnCode);
    return false;
  }
  return true;
}

bool WireguardUtilsLinux::configureInterface(const InterfaceConfig& config) {
  /*
   * Set conf:
   * - sets name of device
   * - sets public key on device
   * - adds peer to device
   * -- sets private key on peer
   * -- sets endpoint on peer
   * -- sets allowed ips on peer
   */

  wg_device* device = static_cast<wg_device*>(calloc(1, sizeof(*device)));
  if (!device) {
    logger.log() << "Allocation failure";
    return false;
  }
  auto guard = qScopeGuard([&] { wg_free_device(device); });

  // Name
  strncpy(device->name, WG_INTERFACE, IFNAMSIZ);
  // Private Key
  wg_key_from_base64(device->private_key, config.m_privateKey.toLocal8Bit());
  // Peer
  if (!buildPeerForDevice(device, config)) {
    logger.log() << "Failed to create peer.";
    return false;
  }
  // Set/update device
  device->flags =
      (wg_device_flags)(WGPEER_HAS_PUBLIC_KEY | WGDEVICE_HAS_PRIVATE_KEY |
                        WGDEVICE_REPLACE_PEERS);
  if (wg_set_device(device) != 0) {
    logger.log() << "Failed to set the new peer";
    return false;
  }
  return true;
}

bool WireguardUtilsLinux::deleteInterface() {
  int returnCode = wg_del_device(WG_INTERFACE);
  if (returnCode != 0) {
    qWarning("Deleting interface `%s` failed with return code: %d",
             WG_INTERFACE, returnCode);
    return false;
  }
  return true;
}

WireguardUtils::peerBytes WireguardUtilsLinux::getThroughputForInterface() {
  uint64_t txBytes = 0;
  uint64_t rxBytes = 0;
  wg_device* device = nullptr;
  wg_peer* peer = nullptr;
  peerBytes pb = {0, 0};
  if (wg_get_device(&device, WG_INTERFACE) != 0) {
    qWarning("Unable to get interface `%s`.", WG_INTERFACE);
    return pb;
  }
  wg_for_each_peer(device, peer) {
    txBytes += peer->tx_bytes;
    rxBytes += peer->rx_bytes;
  }
  wg_free_device(device);
  pb.rxBytes = double(rxBytes);
  pb.txBytes = double(txBytes);
  return pb;
}

// PRIVATE METHODS
QStringList WireguardUtilsLinux::currentInterfaces() {
  char* deviceNames = wg_list_device_names();
  QStringList devices;
  if (!deviceNames) {
    return devices;
  }
  char* deviceName;
  size_t len;
  wg_for_each_device_name(deviceNames, deviceName, len) {
    devices.append(deviceName);
  }
  free(deviceNames);
  return devices;
}

bool WireguardUtilsLinux::buildPeerForDevice(wg_device* device,
                                             const InterfaceConfig& config) {
  Q_ASSERT(device);
  wg_peer* peer = static_cast<wg_peer*>(calloc(1, sizeof(*peer)));
  if (!peer) {
    logger.log() << "Allocation failure";
    return false;
  }
  device->first_peer = device->last_peer = peer;

  // Public Key
  wg_key_from_base64(peer->public_key, config.m_serverPublicKey.toLocal8Bit());
  // Endpoint
  if (!setPeerEndpoint(&peer->endpoint.addr, config.m_serverIpv4AddrIn,
                       config.m_serverPort)) {
    return false;
  }
  // Allowed IPs
  if (!setAllowedIpsOnPeer(peer, config.m_allowedIPAddressRanges)) {
    logger.log() << "Failed to set allowed IPs on Peer";
    return false;
  }
  return true;
}

bool WireguardUtilsLinux::setPeerEndpoint(struct sockaddr* peerEndpoint,
                                          const QString& address, int port) {
  QString portString = QString::number(port);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  struct addrinfo* resolved = nullptr;
  auto guard = qScopeGuard([&] { freeaddrinfo(resolved); });
  int retries = 15;

  for (unsigned int timeout = 1000000;;
       timeout = std::min((unsigned int)20000000, timeout * 6 / 5)) {
    int rv = getaddrinfo(address.toLocal8Bit(), portString.toLocal8Bit(),
                         &hints, &resolved);
    if (!rv) {
      break;
    }

    /* The set of return codes that are "permanent failures". All other
     * possibilities are potentially transient.
     *
     * This is according to https://sourceware.org/glibc/wiki/NameResolver which
     * states: "From the perspective of the application that calls getaddrinfo()
     * it perhaps doesn't matter that much since EAI_FAIL, EAI_NONAME and
     * EAI_NODATA are all permanent failure codes and the causes are all
     * permanent failures in the sense that there is no point in retrying
     * later."
     *
     * So this is what we do, except FreeBSD removed EAI_NODATA some time ago,
     * so that's conditional.
     */
    if (rv == EAI_NONAME || rv == EAI_FAIL ||
#ifdef EAI_NODATA
        rv == EAI_NODATA ||
#endif
        (retries >= 0 && !retries--)) {
      logger.log() << "Failed to resolve the address endpoint";
      return false;
    }

    logger.log() << "Trying again in" << (timeout / 1000000.0) << "seconds";
    usleep(timeout);
  }

  if ((resolved->ai_family == AF_INET &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in)) ||
      (resolved->ai_family == AF_INET6 &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in6))) {
    memcpy(peerEndpoint, resolved->ai_addr, resolved->ai_addrlen);
    return true;
  }

  logger.log() << "Invalid endpoint" << address;
  return false;
}

bool WireguardUtilsLinux::setAllowedIpsOnPeer(
    wg_peer* peer, QList<IPAddressRange> allowedIPAddressRanges) {
  Q_ASSERT(peer);

  for (const IPAddressRange& ip : allowedIPAddressRanges) {
    wg_allowedip* allowedip =
        static_cast<wg_allowedip*>(calloc(1, sizeof(*allowedip)));
    if (!allowedip) {
      logger.log() << "Allocation failure";
      return false;
    }

    if (!peer->first_allowedip) {
      peer->first_allowedip = allowedip;
    } else {
      peer->last_allowedip->next_allowedip = allowedip;
    }

    peer->last_allowedip = allowedip;
    allowedip->cidr = ip.range();

    bool ok = false;
    if (ip.type() == IPAddressRange::IPv4) {
      allowedip->family = AF_INET;
      ok = inet_pton(AF_INET, ip.ipAddress().toLocal8Bit(), &allowedip->ip4) ==
           1;
    } else if (ip.type() == IPAddressRange::IPv6) {
      allowedip->family = AF_INET6;
      ok = inet_pton(AF_INET6, ip.ipAddress().toLocal8Bit(), &allowedip->ip6) ==
           1;
    } else {
      logger.log() << "Invalid IPAddressRange type";
      return false;
    }
    if (!ok) {
      logger.log() << "Invalid IP address:" << ip.ipAddress();
      return false;
    }
  }

  return true;
}