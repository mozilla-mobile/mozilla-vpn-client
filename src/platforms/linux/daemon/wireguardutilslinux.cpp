/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilslinux.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/linux/linuxdependencies.h"

#include <QScopeGuard>

#include <arpa/inet.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <mntent.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// Import wireguard C library for Linux
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#include "../../linux/netfilter/netfilter.h"
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

static void nlmsg_append_attr(char* buf, size_t maxlen, int attrtype,
                              const void* attrdata, size_t attrlen);
static void nlmsg_append_attr32(char* buf, size_t maxlen, int attrtype,
                                uint32_t value);

namespace {
Logger logger(LOG_LINUX, "WireguardUtilsLinux");

void NetfilterLogger(int level, const char* msg) {
  Q_UNUSED(level);
  logger.log() << "NetfilterGo:" << msg;
}
}  // namespace

WireguardUtilsLinux::WireguardUtilsLinux(QObject* parent)
    : WireguardUtils(parent) {
  MVPN_COUNT_CTOR(WireguardUtilsLinux);
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

  /* Create control groups for split tunnelling */
  m_cgroups = LinuxDependencies::findCgroupPath("net_cls");
  if (!m_cgroups.isNull()) {
    if (!setupCgroupClass(m_cgroups + VPN_EXCLUDE_CGROUP,
                          VPN_EXCLUDE_CLASS_ID)) {
      m_cgroups.clear();
    } else if (!setupCgroupClass(m_cgroups + VPN_BLOCK_CGROUP,
                                 VPN_BLOCK_CLASS_ID)) {
      m_cgroups.clear();
    }
  }

  logger.log() << "WireguardUtilsLinux created.";
}

WireguardUtilsLinux::~WireguardUtilsLinux() {
  MVPN_COUNT_DTOR(WireguardUtilsLinux);
  NetfilterRemoveTables();
  if (m_nlsock >= 0) {
    close(m_nlsock);
  }
  logger.log() << "WireguardUtilsLinux destroyed.";
}

bool WireguardUtilsLinux::interfaceExists() {
  // As currentInterfaces only gets wireguard interfaces, this method
  // also confirms an interface as being a wireguard interface.
  return currentInterfaces().contains(WG_INTERFACE);
};

bool WireguardUtilsLinux::addInterface(const InterfaceConfig& config) {
  int returnCode = wg_add_device(WG_INTERFACE);
  if (returnCode != 0) {
    qWarning("Adding interface `%s` failed with return code: %d", WG_INTERFACE,
             returnCode);
    return false;
  }
  return updateInterface(config);
}

bool WireguardUtilsLinux::updateInterface(const InterfaceConfig& config) {
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
    logger.error() << "Allocation failure";
    return false;
  }
  auto guard = qScopeGuard([&] { wg_free_device(device); });

  // Name
  strncpy(device->name, WG_INTERFACE, IFNAMSIZ);
  // Private Key
  wg_key_from_base64(device->private_key, config.m_privateKey.toLocal8Bit());
  // Peer
  if (!buildPeerForDevice(device, config)) {
    logger.error() << "Failed to create peer.";
    return false;
  }
  // Set/update device
  device->fwmark = WG_FIREWALL_MARK;
  device->flags =
      (wg_device_flags)(WGPEER_HAS_PUBLIC_KEY | WGDEVICE_HAS_PRIVATE_KEY |
                        WGDEVICE_REPLACE_PEERS | WGDEVICE_HAS_FWMARK);
  if (wg_set_device(device) != 0) {
    logger.error() << "Failed to set the new peer";
    return false;
  }
  // Create routing policy rules
  if (!setRouteRules(RTM_NEWRULE,
                     NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK,
                     AF_INET)) {
    return false;
  }
  if (!setRouteRules(RTM_NEWRULE,
                     NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK,
                     AF_INET6)) {
    return false;
  }

  // Configure firewall rules
  GoString goIfname = {.p = device->name, .n = (ptrdiff_t)strlen(device->name)};
  if (NetfilterIfup(goIfname, device->fwmark) != 0) {
    return false;
  }
  if (!m_cgroups.isNull()) {
    NetfilterMarkCgroup(VPN_EXCLUDE_CLASS_ID, device->fwmark);
    NetfilterBlockCgroup(VPN_BLOCK_CLASS_ID);
  }
  if (config.m_ipv6Enabled) {
    int slashPos = config.m_deviceIpv6Address.indexOf('/');
    GoString goIpv6Address = {.p = qPrintable(config.m_deviceIpv6Address),
                              .n = config.m_deviceIpv6Address.length()};
    if (slashPos != -1) {
      goIpv6Address.n = slashPos;
    }
    NetfilterIsolateIpv6(goIfname, goIpv6Address);
  }
  return true;
}

bool WireguardUtilsLinux::deleteInterface() {
  // Clear firewall rules
  NetfilterClearTables();

  // Clear routing policy rules
  if (!setRouteRules(RTM_DELRULE, NLM_F_REQUEST | NLM_F_ACK, AF_INET)) {
    return false;
  }
  if (!setRouteRules(RTM_DELRULE, NLM_F_REQUEST | NLM_F_ACK, AF_INET6)) {
    return false;
  }

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

bool WireguardUtilsLinux::addRoutePrefix(const IPAddressRange& prefix) {
  constexpr size_t rtm_max_size = sizeof(struct rtmsg) +
                                  2 * RTA_SPACE(sizeof(uint32_t)) +
                                  RTA_SPACE(sizeof(struct in6_addr));

  int index = if_nametoindex(WG_INTERFACE);
  if (index <= 0) {
    logger.error() << "if_nametoindex() failed:" << strerror(errno);
    return false;
  }

  char buf[NLMSG_SPACE(rtm_max_size)];
  struct nlmsghdr* nlmsg = (struct nlmsghdr*)buf;
  struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlmsg);

  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  nlmsg->nlmsg_type = RTM_NEWROUTE;
  nlmsg->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
  nlmsg->nlmsg_pid = getpid();
  nlmsg->nlmsg_seq = m_nlseq++;
  rtm->rtm_dst_len = prefix.range();
  rtm->rtm_type = RTN_UNICAST;
  rtm->rtm_protocol = RTPROT_BOOT;
  rtm->rtm_scope = RT_SCOPE_UNIVERSE;
  rtm->rtm_table = RT_TABLE_UNSPEC;
  nlmsg_append_attr32(buf, sizeof(buf), RTA_TABLE, WG_ROUTE_TABLE);

  const char* addrstring = qPrintable(prefix.ipAddress());
  if (prefix.type() == IPAddressRange::IPv6) {
    struct in6_addr addrbuf;
    rtm->rtm_family = AF_INET6;
    if (inet_pton(AF_INET6, addrstring, &addrbuf) != 1) {
      logger.error() << "Invalid IPv6 destination prefix";
      return false;
    }
    nlmsg_append_attr(buf, sizeof(buf), RTA_DST, &addrbuf, sizeof(addrbuf));
  } else {
    struct in_addr addrbuf;
    rtm->rtm_family = AF_INET;
    int err = inet_pton(AF_INET, addrstring, &addrbuf);
    if (err != 1) {
      logger.error() << "Invalid IPv4 destination prefix";
      return false;
    }
    nlmsg_append_attr(buf, sizeof(buf), RTA_DST, &addrbuf, sizeof(addrbuf));
  }
  nlmsg_append_attr32(buf, sizeof(buf), RTA_OIF, if_nametoindex(WG_INTERFACE));

  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  size_t result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0,
                         (struct sockaddr*)&nladdr, sizeof(nladdr));
  return (result == nlmsg->nlmsg_len);
}

void WireguardUtilsLinux::flushRoutes() {
  // We should probably implement this to ward off potential corruption in the
  // routing table after silent server switching. It doesn't *really* affect
  // Linux since we use the firewall mark to direct packets, but in theory it
  // could break the captive portal check.
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
    logger.error() << "Allocation failure";
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
    logger.error() << "Failed to set allowed IPs on Peer";
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
    memcpy(peerEndpoint, resolved->ai_addr, resolved->ai_addrlen);
    return true;
  }

  logger.error() << "Invalid endpoint" << address;
  return false;
}

bool WireguardUtilsLinux::setAllowedIpsOnPeer(
    wg_peer* peer, QList<IPAddressRange> allowedIPAddressRanges) {
  Q_ASSERT(peer);

  for (const IPAddressRange& ip : allowedIPAddressRanges) {
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
    allowedip->cidr = ip.range();

    QString ipstring = ip.ipAddress();
    if (ip.type() == IPAddressRange::IPv4) {
      allowedip->family = AF_INET;
      if (inet_pton(AF_INET, qPrintable(ipstring), &allowedip->ip4) != 1) {
        logger.error() << "Invalid IPv4 address:" << ip.ipAddress();
        return false;
      }
    } else if (ip.type() == IPAddressRange::IPv6) {
      allowedip->family = AF_INET6;
      if (inet_pton(AF_INET6, qPrintable(ipstring), &allowedip->ip6) != 1) {
        logger.error() << "Invalid IPv6 address:" << ip.ipAddress();
        return false;
      }
    } else {
      logger.error() << "Invalid IPAddressRange type";
      return false;
    }
  }

  return true;
}

static void nlmsg_append_attr(char* buf, size_t maxlen, int attrtype,
                              const void* attrdata, size_t attrlen) {
  struct nlmsghdr* nlmsg = (struct nlmsghdr*)buf;
  size_t newlen = NLMSG_ALIGN(nlmsg->nlmsg_len) + RTA_SPACE(attrlen);
  if (newlen <= maxlen) {
    struct rtattr* attr = (struct rtattr*)(buf + NLMSG_ALIGN(nlmsg->nlmsg_len));
    attr->rta_type = attrtype;
    attr->rta_len = RTA_LENGTH(attrlen);
    memcpy(RTA_DATA(attr), attrdata, attrlen);
    nlmsg->nlmsg_len = newlen;
  }
}

static void nlmsg_append_attr32(char* buf, size_t maxlen, int attrtype,
                                uint32_t value) {
  nlmsg_append_attr(buf, maxlen, attrtype, &value, sizeof(value));
}

bool WireguardUtilsLinux::setRouteRules(int action, int flags, int addrfamily) {
  constexpr size_t fib_max_size =
      sizeof(struct fib_rule_hdr) + 2 * RTA_SPACE(sizeof(uint32_t));

  char buf[NLMSG_SPACE(fib_max_size)];
  struct nlmsghdr* nlmsg = (struct nlmsghdr*)buf;
  struct fib_rule_hdr* rule = (struct fib_rule_hdr*)NLMSG_DATA(nlmsg);
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
  nlmsg_append_attr32(buf, sizeof(buf), FRA_FWMARK, WG_FIREWALL_MARK);
  nlmsg_append_attr32(buf, sizeof(buf), FRA_TABLE, WG_ROUTE_TABLE);
  ssize_t result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0,
                          (struct sockaddr*)&nladdr, sizeof(nladdr));
  if (result != nlmsg->nlmsg_len) {
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
  nlmsg_append_attr32(buf, sizeof(buf), FRA_SUPPRESS_PREFIXLEN, 0);
  result = sendto(m_nlsock, buf, nlmsg->nlmsg_len, 0, (struct sockaddr*)&nladdr,
                  sizeof(nladdr));
  if (result != nlmsg->nlmsg_len) {
    return false;
  }

  return true;
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
    struct nlmsgerr* err = (struct nlmsgerr*)NLMSG_DATA(nlmsg);
    if (err->error != 0) {
      logger.log() << "Netlink request failed:" << strerror(-err->error);
    }
    nlmsg = NLMSG_NEXT(nlmsg, len);
  }
}

// static
bool WireguardUtilsLinux::setupCgroupClass(const QString& path,
                                           unsigned long classid) {
  logger.log() << "Creating control group:" << path;
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

QString WireguardUtilsLinux::getExcludeCgroup() const {
  if (m_cgroups.isNull()) {
    return QString();
  }
  return m_cgroups + VPN_EXCLUDE_CGROUP;
}

QString WireguardUtilsLinux::getBlockCgroup() const {
  if (m_cgroups.isNull()) {
    return QString();
  }
  return m_cgroups + VPN_BLOCK_CGROUP;
}
