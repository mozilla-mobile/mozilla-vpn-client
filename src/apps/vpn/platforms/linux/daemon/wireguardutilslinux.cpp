/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilslinux.h"

#include <arpa/inet.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <mntent.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QFile>
#include <QHostAddress>
#include <QScopeGuard>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/linux/linuxdependencies.h"

// Import wireguard C library for Linux
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#include "netfilter.h"
#if defined(__cplusplus)
}
#endif
// End import wireguard

/* Packets sent outside the VPN need to be marked for the routing policy
 * to direct them appropriately. The value of the mark and the table ID
 * aren't important, so long as they are unique.
 */
constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr uint32_t WG_ROUTE_TABLE = 0xca6c;

/* Traffic classifiers can be used to mark packets which should be either
 * excluded from the VPN tunnel, or blocked entirely. The values of these
 * classifiers aren't important so long as they are unique.
 */
constexpr const char* VPN_EXCLUDE_CGROUP = "/mozvpn.exclude";
constexpr const char* VPN_BLOCK_CGROUP = "/mozvpn.block";
constexpr uint32_t VPN_EXCLUDE_CLASS_ID = 0x00110011;
constexpr uint32_t VPN_BLOCK_CLASS_ID = 0x00220022;

static void nlmsg_append_attr(struct nlmsghdr* nlmsg, size_t maxlen,
                              int attrtype, const void* attrdata,
                              size_t attrlen);
static void nlmsg_append_attr32(struct nlmsghdr* nlmsg, size_t maxlen,
                                int attrtype, uint32_t value);

namespace {
Logger logger("WireguardUtilsLinux");

void NetfilterLogger(int level, const char* msg) {
  Q_UNUSED(level);
  logger.debug() << "NetfilterGo:" << msg;
}
}  // namespace

WireguardUtilsLinux::WireguardUtilsLinux(QObject* parent)
    : WireguardUtils(parent) {
  MZ_COUNT_CTOR(WireguardUtilsLinux);
  NetfilterSetLogger((GoUintptr)&NetfilterLogger);
  NetfilterCreateTables();

  m_nlsock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (m_nlsock < 0) {
    logger.warning() << "Failed to create netlink socket:" << strerror(errno);
  }

  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  nladdr.nl_pid = getpid();
  if (bind(m_nlsock, (struct sockaddr*)&nladdr, sizeof(nladdr)) != 0) {
    logger.warning() << "Failed to bind netlink socket:" << strerror(errno);
  }

  m_notifier = new QSocketNotifier(m_nlsock, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this,
          &WireguardUtilsLinux::nlsockReady);

  // Most kernels cannot simultaneously support traffic classification with
  // both the net_cls (v1) and unified (v2) cgroups simultaneously. If both
  // are present, the net_cls traffic classifiers take priority.
  //
  // In this situation, you will likely see this kernel log warning:
  //   cgroup: disabling cgroup2 socket matching due to net_prio or net_cls
  //   activation
  m_cgroupNetClass = LinuxDependencies::findCgroupPath("net_cls");
  m_cgroupUnified = LinuxDependencies::findCgroup2Path();
  if (!m_cgroupNetClass.isNull()) {
    if (setupCgroupClass(m_cgroupNetClass + VPN_EXCLUDE_CGROUP,
                         VPN_EXCLUDE_CLASS_ID)) {
      logger.info() << "Setup split tunneling with net_cls cgroups (v1)";
      m_cgroupVersion = 1;
    }
  }
  // Otherwise, try to use unified cgroups (v2)
  else if ((m_cgroupVersion == 0) && !m_cgroupUnified.isNull()) {
    logger.info() << "Setup split tunneling with unified cgroups (v2)";
    m_cgroupVersion = 2;
  } else {
    logger.warning() << "Unable to setup split tunneling: no supported cgroups";
  }

  logger.debug() << "WireguardUtilsLinux created.";
}

WireguardUtilsLinux::~WireguardUtilsLinux() {
  MZ_COUNT_DTOR(WireguardUtilsLinux);
  NetfilterRemoveTables();
  if (m_nlsock >= 0) {
    close(m_nlsock);
  }
  logger.debug() << "WireguardUtilsLinux destroyed.";
}

bool WireguardUtilsLinux::interfaceExists() {
  // As currentInterfaces only gets wireguard interfaces, this method
  // also confirms an interface as being a wireguard interface.
  return currentInterfaces().contains(WG_INTERFACE);
};

bool WireguardUtilsLinux::addInterface(const InterfaceConfig& config) {
  int code = wg_add_device(WG_INTERFACE);
  if (code != 0) {
    logger.error() << "Adding interface failed:" << strerror(-code);
    return false;
  }

  wg_device* device = static_cast<wg_device*>(calloc(1, sizeof(*device)));
  if (!device) {
    logger.error() << "Allocation failure";
    return false;
  }
  auto guard = qScopeGuard([&] { wg_free_device(device); });

  // Name
  strncpy(device->name, WG_INTERFACE, IFNAMSIZ);
  // Private Key
  wg_key_from_base64(device->private_key, config.m_privateKey.toLocal8Bit());

  // Set/update device
  device->fwmark = WG_FIREWALL_MARK;
  device->flags = (wg_device_flags)(
      WGDEVICE_HAS_PRIVATE_KEY | WGDEVICE_REPLACE_PEERS | WGDEVICE_HAS_FWMARK);
  if (wg_set_device(device) != 0) {
    logger.error() << "Failed to setup the device";
    return false;
  }

  // Create routing policy rules
  if (!rtmSendRule(RTM_NEWRULE,
                   NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK,
                   AF_INET)) {
    return false;
  }
  if (!rtmSendRule(RTM_NEWRULE,
                   NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK,
                   AF_INET6)) {
    return false;
  }

  // Configure firewall rules
  GoString goIfname = {.p = device->name, .n = (ptrdiff_t)strlen(device->name)};
  if (NetfilterIfup(goIfname, device->fwmark) != 0) {
    return false;
  }
  if (m_cgroupVersion == 1) {
    NetfilterMarkCgroupV1(VPN_EXCLUDE_CLASS_ID);
  }

  int slashPos = config.m_deviceIpv6Address.indexOf('/');
  GoString goIpv6Address = {.p = qPrintable(config.m_deviceIpv6Address),
                            .n = config.m_deviceIpv6Address.length()};
  if (slashPos != -1) {
    goIpv6Address.n = slashPos;
  }
  NetfilterIsolateIpv6(goIfname, goIpv6Address);

  return true;
}

bool WireguardUtilsLinux::updatePeer(const InterfaceConfig& config) {
  wg_device* device = static_cast<wg_device*>(calloc(1, sizeof(*device)));
  if (!device) {
    logger.error() << "Allocation failure";
    return false;
  }
  auto guard = qScopeGuard([&] { wg_free_device(device); });

  wg_peer* peer = static_cast<wg_peer*>(calloc(1, sizeof(*peer)));
  if (!peer) {
    logger.error() << "Allocation failure";
    return false;
  }
  device->first_peer = device->last_peer = peer;

  logger.debug() << "Adding peer" << logger.keys(config.m_serverPublicKey);

  // Public Key
  wg_key_from_base64(peer->public_key, qPrintable(config.m_serverPublicKey));
  // Endpoint
  if (!setPeerEndpoint(&peer->endpoint.addr, config.m_serverIpv4AddrIn,
                       config.m_serverPort)) {
    logger.error() << "Failed to set peer endpoint for" << config.m_hopType
                   << "hop";
    return false;
  }

  // HACK: We are running into a crash on Linux due to the address list being
  // *WAAAY* too long, which we aren't really using anways since the routing
  // policy rules are doing all the work for us anyways.
  //
  // To work around the issue, just set default routes for the exit hop.
  if ((config.m_hopType == "single") || (config.m_hopType == "exit")) {
    if (!config.m_deviceIpv4Address.isNull()) {
      addPeerPrefix(peer, IPAddress("0.0.0.0/0"));
    }
    if (!config.m_deviceIpv6Address.isNull()) {
      addPeerPrefix(peer, IPAddress("::/0"));
    }
  } else if (config.m_hopType == "entry") {
    // Add allowed addresses for the multihop entry server(s)
    for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
      bool ok = addPeerPrefix(peer, ip);
      if (!ok) {
        logger.error() << "Invalid IP address:" << ip.toString();
        return false;
      }

      // Direct multihop exit destinations to use the wireguard table.
      int flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
      rtmIncludePeer(RTM_NEWRULE, flags, ip);
    }
  }

  // Update the firewall to mark inbound traffic from the server.
  GoString goAddress = {.p = qPrintable(config.m_serverIpv4AddrIn),
                        .n = (ptrdiff_t)config.m_serverIpv4AddrIn.length()};
  NetfilterMarkInbound(goAddress, config.m_serverPort);

  // Set/update peer
  strncpy(device->name, WG_INTERFACE, IFNAMSIZ);
  device->flags = (wg_device_flags)0;
  peer->persistent_keepalive_interval = WG_KEEPALIVE_PERIOD;
  peer->flags =
      (wg_peer_flags)(WGPEER_HAS_PUBLIC_KEY | WGPEER_REPLACE_ALLOWEDIPS |
                      WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL);
  if (wg_set_device(device) != 0) {
    logger.error() << "Failed to set the new peer" << config.m_hopType << "hop";
    return false;
  }

  return true;
}

bool WireguardUtilsLinux::deletePeer(const InterfaceConfig& config) {
  wg_device* device = static_cast<wg_device*>(calloc(1, sizeof(*device)));
  if (!device) {
    logger.error() << "Allocation failure";
    return false;
  }
  auto guard = qScopeGuard([&] { wg_free_device(device); });

  wg_peer* peer = static_cast<wg_peer*>(calloc(1, sizeof(*peer)));
  if (!peer) {
    logger.error() << "Allocation failure";
    return false;
  }
  device->first_peer = device->last_peer = peer;

  logger.debug() << "Removing peer" << logger.keys(config.m_serverPublicKey);

  // Public Key
  peer->flags = (wg_peer_flags)(WGPEER_HAS_PUBLIC_KEY | WGPEER_REMOVE_ME);
  wg_key_from_base64(peer->public_key, qPrintable(config.m_serverPublicKey));

  // Clear routing policy tweaks for multihop.
  if (config.m_hopType != "entry") {
    for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
      rtmIncludePeer(RTM_DELRULE, NLM_F_REQUEST | NLM_F_ACK, ip);
    }
  }

  // Clear firewall settings for this server.
  GoString goAddress = {.p = qPrintable(config.m_serverIpv4AddrIn),
                        .n = (ptrdiff_t)config.m_serverIpv4AddrIn.length()};
  NetfilterClearInbound(goAddress);

  // Set/update device
  strncpy(device->name, WG_INTERFACE, IFNAMSIZ);
  device->flags = (wg_device_flags)0;
  if (wg_set_device(device) != 0) {
    logger.error() << "Failed to remove the peer";
    return false;
  }

  return true;
}

bool WireguardUtilsLinux::deleteInterface() {
  // Clear firewall rules
  NetfilterClearTables();

  // Clear routing policy rules
  if (!rtmSendRule(RTM_DELRULE, NLM_F_REQUEST | NLM_F_ACK, AF_INET)) {
    return false;
  }
  if (!rtmSendRule(RTM_DELRULE, NLM_F_REQUEST | NLM_F_ACK, AF_INET6)) {
    return false;
  }

  // Delete the interface
  int returnCode = wg_del_device(WG_INTERFACE);
  if (returnCode != 0) {
    logger.error() << "Deleting interface failed:" << strerror(-returnCode);
    return false;
  }

  return true;
}

QList<WireguardUtils::PeerStatus> WireguardUtilsLinux::getPeerStatus() {
  wg_device* device = nullptr;
  wg_peer* peer = nullptr;
  QList<WireguardUtils::PeerStatus> peerList;

  if (wg_get_device(&device, WG_INTERFACE) != 0) {
    logger.warning() << "Unable to get stats for" << WG_INTERFACE;
    return peerList;
  }

  wg_for_each_peer(device, peer) {
    PeerStatus status;
    wg_key_b64_string keystring;
    wg_key_to_base64(keystring, peer->public_key);
    status.m_pubkey = QString(keystring);
    status.m_handshake = peer->last_handshake_time.tv_sec * 1000;
    status.m_handshake += peer->last_handshake_time.tv_nsec / 1000000;
    status.m_txBytes = peer->tx_bytes;
    status.m_rxBytes = peer->rx_bytes;
    logger.debug() << "found" << logger.keys(status.m_pubkey) << "handshake"
                   << peer->last_handshake_time.tv_sec;
    peerList.append(status);
  }
  wg_free_device(device);
  return peerList;
}

bool WireguardUtilsLinux::updateRoutePrefix(const IPAddress& prefix) {
  logger.debug() << "Adding route to" << prefix.toString();

  const int flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
  return rtmSendRoute(RTM_NEWROUTE, flags, RTN_UNICAST, prefix);
}

bool WireguardUtilsLinux::deleteRoutePrefix(const IPAddress& prefix) {
  logger.debug() << "Removing route to" << logger.sensitive(prefix.toString());

  const int flags = NLM_F_REQUEST | NLM_F_ACK;
  return rtmSendRoute(RTM_DELROUTE, flags, RTN_UNICAST, prefix);
}

bool WireguardUtilsLinux::addExclusionRoute(const IPAddress& prefix) {
  logger.debug() << "Adding exclusion route for"
                 << logger.sensitive(prefix.toString());
  const int flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
  return rtmSendRoute(RTM_NEWROUTE, flags, RTN_THROW, prefix);
}

bool WireguardUtilsLinux::deleteExclusionRoute(const IPAddress& prefix) {
  logger.debug() << "Removing exclusion route for"
                 << logger.sensitive(prefix.toString());
  const int flags = NLM_F_REQUEST | NLM_F_ACK;
  return rtmSendRoute(RTM_DELROUTE, flags, RTN_THROW, prefix);
}

bool WireguardUtilsLinux::rtmSendRoute(int action, int flags, int type,
                                       const IPAddress& prefix) {
  constexpr size_t rtm_max_size = sizeof(struct rtmsg) +
                                  2 * RTA_SPACE(sizeof(uint32_t)) +
                                  RTA_SPACE(sizeof(struct in6_addr));
  wg_allowedip ip;
  if (!buildAllowedIp(&ip, prefix)) {
    logger.warning() << "Invalid destination prefix";
    return false;
  }

  char buf[NLMSG_SPACE(rtm_max_size)];
  struct nlmsghdr* nlmsg = reinterpret_cast<struct nlmsghdr*>(buf);
  struct rtmsg* rtm = static_cast<struct rtmsg*>(NLMSG_DATA(nlmsg));

  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  nlmsg->nlmsg_type = action;
  nlmsg->nlmsg_flags = flags;
  nlmsg->nlmsg_pid = getpid();
  nlmsg->nlmsg_seq = m_nlseq++;
  rtm->rtm_dst_len = ip.cidr;
  rtm->rtm_family = ip.family;
  rtm->rtm_type = type;
  rtm->rtm_table = RT_TABLE_UNSPEC;
  rtm->rtm_protocol = RTPROT_BOOT;
  rtm->rtm_scope = RT_SCOPE_UNIVERSE;

  // Place all routes in their own table.
  nlmsg_append_attr32(nlmsg, sizeof(buf), RTA_TABLE, WG_ROUTE_TABLE);

  if (rtm->rtm_family == AF_INET6) {
    nlmsg_append_attr(nlmsg, sizeof(buf), RTA_DST, &ip.ip6, sizeof(ip.ip6));
  } else {
    nlmsg_append_attr(nlmsg, sizeof(buf), RTA_DST, &ip.ip4, sizeof(ip.ip4));
  }

  if (rtm->rtm_type == RTN_UNICAST) {
    int index = if_nametoindex(WG_INTERFACE);
    if (index <= 0) {
      logger.error() << "if_nametoindex() failed:" << strerror(errno);
      return false;
    }
    nlmsg_append_attr32(nlmsg, sizeof(buf), RTA_OIF, index);
  }

  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  size_t result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0,
                         (struct sockaddr*)&nladdr, sizeof(nladdr));
  return (result == nlmsg->nlmsg_len);
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

bool WireguardUtilsLinux::setPeerEndpoint(struct sockaddr* sa,
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
      logger.error() << "Failed to resolve the address endpoint";
      return false;
    }

    logger.warning() << "Trying again in" << (timeout / 1000000.0) << "seconds";
    usleep(timeout);
  }

  if ((resolved->ai_family == AF_INET &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in)) ||
      (resolved->ai_family == AF_INET6 &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in6))) {
    memcpy(sa, resolved->ai_addr, resolved->ai_addrlen);
    return true;
  }

  logger.error() << "Invalid endpoint" << address;
  return false;
}

bool WireguardUtilsLinux::addPeerPrefix(wg_peer* peer,
                                        const IPAddress& prefix) {
  Q_ASSERT(peer);

  wg_allowedip* allowedip =
      static_cast<wg_allowedip*>(calloc(1, sizeof(*allowedip)));
  if (!allowedip) {
    logger.error() << "Allocation failure";
    return false;
  }

  if (!peer->first_allowedip) {
    peer->first_allowedip = allowedip;
  } else {
    peer->last_allowedip->next_allowedip = allowedip;
  }
  peer->last_allowedip = allowedip;

  return buildAllowedIp(allowedip, prefix);
}

static void nlmsg_append_attr(struct nlmsghdr* nlmsg, size_t maxlen,
                              int attrtype, const void* attrdata,
                              size_t attrlen) {
  size_t newlen = NLMSG_ALIGN(nlmsg->nlmsg_len) + RTA_SPACE(attrlen);
  if (newlen <= maxlen) {
    char* buf = reinterpret_cast<char*>(nlmsg) + NLMSG_ALIGN(nlmsg->nlmsg_len);
    struct rtattr* attr = reinterpret_cast<struct rtattr*>(buf);
    attr->rta_type = attrtype;
    attr->rta_len = RTA_LENGTH(attrlen);
    memcpy(RTA_DATA(attr), attrdata, attrlen);
    nlmsg->nlmsg_len = newlen;
  }
}

static void nlmsg_append_attr32(struct nlmsghdr* nlmsg, size_t maxlen,
                                int attrtype, uint32_t value) {
  nlmsg_append_attr(nlmsg, maxlen, attrtype, &value, sizeof(value));
}

bool WireguardUtilsLinux::rtmSendRule(int action, int flags, int addrfamily) {
  constexpr size_t fib_max_size =
      sizeof(struct fib_rule_hdr) + 2 * RTA_SPACE(sizeof(uint32_t));

  char buf[NLMSG_SPACE(fib_max_size)];
  struct nlmsghdr* nlmsg = reinterpret_cast<struct nlmsghdr*>(buf);
  struct fib_rule_hdr* rule =
      static_cast<struct fib_rule_hdr*>(NLMSG_DATA(nlmsg));
  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;

  /* Create a routing policy rule to select the wireguard routing table for
   * unmarked packets. This is equivalent to:
   *     ip rule add not fwmark $WG_FIREWALL_MARK table $WG_ROUTE_TABLE
   */
  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct fib_rule_hdr));
  nlmsg->nlmsg_type = action;
  nlmsg->nlmsg_flags = flags;
  nlmsg->nlmsg_pid = getpid();
  nlmsg->nlmsg_seq = m_nlseq++;
  rule->family = addrfamily;
  rule->table = RT_TABLE_UNSPEC;
  rule->action = FR_ACT_TO_TBL;
  rule->flags = FIB_RULE_INVERT;
  nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_FWMARK, WG_FIREWALL_MARK);
  nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_TABLE, WG_ROUTE_TABLE);
  ssize_t result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0,
                          (struct sockaddr*)&nladdr, sizeof(nladdr));
  if (result != static_cast<ssize_t>(nlmsg->nlmsg_len)) {
    return false;
  }

  /* Create a routing policy rule to suppress zero-length prefix lookups from
   * in the main routing table. This is equivalent to:
   *     ip rule add table main suppress_prefixlength 0
   */
  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct fib_rule_hdr));
  nlmsg->nlmsg_type = action;
  nlmsg->nlmsg_flags = flags;
  nlmsg->nlmsg_pid = getpid();
  nlmsg->nlmsg_seq = m_nlseq++;
  rule->family = addrfamily;
  rule->table = RT_TABLE_MAIN;
  rule->action = FR_ACT_TO_TBL;
  rule->flags = 0;
  nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_SUPPRESS_PREFIXLEN, 0);
  result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0, (struct sockaddr*)&nladdr,
                  sizeof(nladdr));
  if (result != static_cast<ssize_t>(nlmsg->nlmsg_len)) {
    return false;
  }

  return true;
}

bool WireguardUtilsLinux::rtmIncludePeer(int action, int flags,
                                         const IPAddress& prefix) {
  constexpr size_t fib_max_size = sizeof(struct fib_rule_hdr) +
                                  RTA_SPACE(sizeof(struct in6_addr)) +
                                  2 * RTA_SPACE(sizeof(quint32));

  char buf[NLMSG_SPACE(fib_max_size)];
  struct nlmsghdr* nlmsg = reinterpret_cast<struct nlmsghdr*>(buf);
  struct fib_rule_hdr* rule =
      static_cast<struct fib_rule_hdr*>(NLMSG_DATA(nlmsg));
  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;

  // Create a routing policy rule to select the wireguard routing table for
  // marked packets matching the destination address. This is equivalent to:
  //    ip rule add fwmark $MARK to $PREFIX table $WG_ROUTE_TABLE
  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct fib_rule_hdr));
  nlmsg->nlmsg_type = action;
  nlmsg->nlmsg_flags = flags;
  nlmsg->nlmsg_pid = getpid();
  nlmsg->nlmsg_seq = m_nlseq++;
  rule->table = RT_TABLE_UNSPEC;
  rule->action = FR_ACT_TO_TBL;
  rule->flags = 0;
  rule->dst_len = prefix.prefixLength();
  nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_FWMARK, WG_FIREWALL_MARK);
  nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_TABLE, WG_ROUTE_TABLE);

  if (prefix.address().protocol() == QAbstractSocket::IPv6Protocol) {
    Q_IPV6ADDR dst = prefix.address().toIPv6Address();
    nlmsg_append_attr(nlmsg, sizeof(buf), FRA_DST, &dst, sizeof(dst));
    rule->family = AF_INET6;
  } else if (prefix.address().protocol() == QAbstractSocket::IPv4Protocol) {
    quint32 dst = prefix.address().toIPv4Address();
    nlmsg_append_attr32(nlmsg, sizeof(buf), FRA_DST, htonl(dst));
    rule->family = AF_INET;
  } else {
    return false;
  }

  ssize_t result =
      sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0,
             reinterpret_cast<struct sockaddr*>(&nladdr), sizeof(nladdr));
  return (result == static_cast<ssize_t>(nlmsg->nlmsg_len));
}

void WireguardUtilsLinux::nlsockReady() {
  char buf[1024];
  ssize_t len = recv(m_nlsock, buf, sizeof(buf), MSG_DONTWAIT);
  if (len <= 0) {
    return;
  }

  struct nlmsghdr* nlmsg = (struct nlmsghdr*)buf;
  while (NLMSG_OK(nlmsg, len)) {
    if (nlmsg->nlmsg_type == NLMSG_DONE) {
      return;
    }
    if (nlmsg->nlmsg_type != NLMSG_ERROR) {
      nlmsg = NLMSG_NEXT(nlmsg, len);
      continue;
    }
    struct nlmsgerr* err = static_cast<struct nlmsgerr*>(NLMSG_DATA(nlmsg));
    if (err->error != 0) {
      logger.debug() << "Netlink request failed:" << strerror(-err->error);
    }
    nlmsg = NLMSG_NEXT(nlmsg, len);
  }
}

// static
bool WireguardUtilsLinux::setupCgroupClass(const QString& path,
                                           unsigned long classid) {
  logger.debug() << "Creating control group:" << path;
  int flags = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
  int err = mkdir(qPrintable(path), flags);
  if ((err < 0) && (errno != EEXIST)) {
    logger.error() << "Failed to create" << path + ":" << strerror(errno);
    return false;
  }

  QString netClassPath = path + "/net_cls.classid";
  FILE* fp = fopen(qPrintable(netClassPath), "w");
  if (!fp) {
    logger.error() << "Failed to set classid:" << strerror(errno);
    return false;
  }
  fprintf(fp, "%lu", classid);
  fclose(fp);
  return true;
}

// static
bool WireguardUtilsLinux::moveCgroupProcs(const QString& src,
                                          const QString& dest) {
  QFile srcProcs(src + "/cgroup.procs");
  FILE* fp = fopen(qPrintable(dest + "/cgroup.procs"), "w");
  if (!fp) {
    return false;
  }
  auto guard = qScopeGuard([&] { fclose(fp); });

  if (!srcProcs.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  while (true) {
    QString line = QString::fromLocal8Bit(srcProcs.readLine());
    if (line.isEmpty()) {
      break;
    }
    fputs(qPrintable(line), fp);
    fflush(fp);
  }
  srcProcs.close();
  return true;
}

void WireguardUtilsLinux::excludeCgroup(const QString& cgroup) {
  logger.info() << "Excluding traffic from" << cgroup;
  if (m_cgroupVersion == 1) {
    // Add all PIDs from the unified cgroup to the net_cls exclusion cgroup.
    moveCgroupProcs(m_cgroupUnified + cgroup,
                    m_cgroupNetClass + VPN_EXCLUDE_CGROUP);
  } else if (m_cgroupVersion == 2) {
    QByteArray cgpath = cgroup.toLocal8Bit();
    GoString goCgroup = {.p = cgpath.constData(),
                         .n = (ptrdiff_t)cgpath.length()};
    NetfilterMarkCgroupV2(goCgroup);
  } else {
    Q_ASSERT(m_cgroupVersion == 0);
  }
}

void WireguardUtilsLinux::resetCgroup(const QString& cgroup) {
  logger.info() << "Permitting traffic from" << cgroup;
  if (m_cgroupVersion == 1) {
    // Add all PIDs from the unified cgroup to the net_cls default cgroup.
    moveCgroupProcs(m_cgroupUnified + cgroup, m_cgroupNetClass);
  } else if (m_cgroupVersion == 2) {
    QByteArray cgpath = cgroup.toLocal8Bit();
    GoString goCgroup = {.p = cgpath.constData(),
                         .n = (ptrdiff_t)cgpath.length()};
    NetfilterResetCgroupV2(goCgroup);
  } else {
    Q_ASSERT(m_cgroupVersion == 0);
  }
}

void WireguardUtilsLinux::resetAllCgroups() {
  logger.info() << "Permitting traffic from all cgroups";
  if (m_cgroupVersion == 1) {
    // Add all PIDs from the net_cls exclusion cgroup to the default cgroup.
    moveCgroupProcs(m_cgroupNetClass + VPN_EXCLUDE_CGROUP, m_cgroupNetClass);
  } else if (m_cgroupVersion == 2) {
    NetfilterResetAllCgroupsV2();
  } else {
    Q_ASSERT(m_cgroupVersion == 0);
  }
}

// static
bool WireguardUtilsLinux::buildAllowedIp(wg_allowedip* ip,
                                         const IPAddress& prefix) {
  const char* addrString = qPrintable(prefix.address().toString());
  if (prefix.type() == QAbstractSocket::IPv4Protocol) {
    ip->family = AF_INET;
    ip->cidr = prefix.prefixLength();
    return inet_pton(AF_INET, addrString, &ip->ip4) == 1;
  }
  if (prefix.type() == QAbstractSocket::IPv6Protocol) {
    ip->family = AF_INET6;
    ip->cidr = prefix.prefixLength();
    return inet_pton(AF_INET6, addrString, &ip->ip6) == 1;
  }
  return false;
}
