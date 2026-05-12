/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "wireguardtunnel.h"

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>

#include <errno.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_utun.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/in_var.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/sysctl.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#import "utils.h"

extern "C" {
#include "wireguard_ffi.h"
};

// Private method in the network framework
extern "C" nw_interface_t nw_interface_create_with_index_and_name(int ifindex, const char *ifname);

@implementation WireguardTunnel {
  int                  m_tunfd;
  dispatch_semaphore_t m_semaphore; 
  NSThread *           m_worker;

  // The routing socket
  int                  m_rtseq;
  int                  m_rtsock;

  // Not used for anything, we just hold them for status generation.
  nw_endpoint_t        m_ipv4gateway;
  nw_endpoint_t        m_ipv6gateway;
}

constexpr const int64_t WG_WORKQUEUE_TIMEOUT = 5LL * 1000000000LL;

static void wgLog(const char* msg) {
  NSLog(@"wg: %s", msg);
}

- (id)init {
  self = [super init];
  set_logging_function(wgLog);

  m_tunfd = -1;
  m_rtseq = 0;
  m_rtsock = -1;

  m_tunfd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
  if (m_tunfd < 0) {
    return nil;
  }

  // Connect to the utun control kernel service.
  struct ctl_info info = {.ctl_name = "com.apple.net.utun_control"};
  int err = ioctl(m_tunfd, CTLIOCGINFO, &info);
  if (err < 0) {
    close(m_tunfd);
    return nil;
  }
  struct sockaddr_ctl addr = {};
  addr.sc_len = sizeof(addr);
  addr.sc_family = AF_SYSTEM;
  addr.ss_sysaddr = AF_SYS_CONTROL;
  addr.sc_id = info.ctl_id;
  addr.sc_unit = 0;
  err = connect(m_tunfd, (struct sockaddr*)&addr, sizeof(addr));
  if (err < 0) {
    close(m_tunfd);
    return nil;
  }

  // Get the tunnel device's name.
  struct ifreq ifr;
  socklen_t ifnamesize = sizeof(ifr.ifr_name);
  err = getsockopt(m_tunfd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifr.ifr_name,
                   &ifnamesize);
  if (err < 0) {
    close(m_tunfd);
    return nil;
  }
  int ifindex = if_nametoindex(ifr.ifr_name);
  self.virtualInterface = nw_interface_create_with_index_and_name(ifindex, ifr.ifr_name);

  // Set a base MTU, it will get updated later.
  ifr.ifr_mtu = IPV6_MMTU;
  if (ioctl(m_tunfd, SIOCSIFMTU, &ifr) == 0) {
    _mtu = IPV6_MMTU;
  } else if (ioctl(m_tunfd, SIOCGIFMTU, &ifr) == 0) {
    _mtu = ifr.ifr_mtu;
  } else {
    _mtu = IPV6_MMTU;
  }

  // Start outbound encryption workers.
  m_semaphore = dispatch_semaphore_create(0);
  m_worker = [[NSThread alloc] initWithTarget:self
                                     selector:@selector(tunnelWorker:)
                                       object:nil];
  [m_worker start];

  return self;
}

- (void) startTunnelWithOptions:(InterfaceConfig *)options 
              completionHandler:(void (^)(NSError *error)) completionHandler {
  m_ipv4gateway = options.serverIpv4Gateway;
  m_ipv6gateway = options.serverIpv6Gateway;

  // Create a routing socket too.
  m_rtsock = socket(PF_ROUTE, SOCK_RAW, 0);
  if (m_rtsock < 0) {
    completionHandler(vpnPosixError(errno, @"routing socket creation failed"));
    return;
  }

  // Assign addresses
  if (NSError *err = [self setTunnelAddress:options.deviceIpv4Addr]) {
    completionHandler(err);
    return;
  }
  if (NSError *err = [self setTunnelAddress:options.deviceIpv6Addr]) {
    completionHandler(err);
    return;
  }

  // Configure routes into the tunnel interface.
  // Note that the default route should set RTF_IFSCOPE so that we
  // leave the real default route untouched.
  for (RoutePrefix* prefix in options.routes) {
    [self rtmSendRoute:RTM_ADD
         toDestination:prefix.destination
            withPrefix:prefix.prefixLength
              andFlags:(prefix.prefixLength == 0) ? RTF_IFSCOPE : 0];
  }

  // Configure the peer
  self.peer = [[WireguardPeer alloc] initWithOptions:options andTunnel:m_tunfd];
  [self.peer startWithOptions:options completionHandler:^(NSError* error){
    NSLog(@"handshake completed");
    if (error) {
      completionHandler(error);
      return;
    }

    // Update the MTU once the socket is open
    //self.mtu = nw_connection_get_maximum_datagram_size(self.peer.connection) - WG_PACKET_OVERHEAD;

    // Bring the device up, if not already done.
    struct ifreq ifr;
    socklen_t ifnamesize = sizeof(ifr.ifr_name);
    int result = getsockopt(m_tunfd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME,
                            ifr.ifr_name, &ifnamesize);
    if (result < 0) {
      completionHandler(vpnPosixError(errno, @"failed to get interface name"));
      return;
    }
    result = ioctl(m_tunfd, SIOCGIFFLAGS, &ifr);
    if (result) {
      completionHandler(vpnPosixError(errno, @"failed to get interface flags"));
      return;
    }
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    result = ioctl(m_tunfd, SIOCSIFFLAGS, &ifr);
    if (result != 0) {
      completionHandler(vpnPosixError(errno, @"failed to set device up"));
      return;
    }

    completionHandler(nil);
  }];
}

- (void) stopTunnelWithReason:(NEProviderStopReason)reason 
            completionHandler:(void (^)()) completionHandler {
  if (!self.peer) {
    [self shutdownTunnel];
    completionHandler();
  }

  [self.peer stopWithReason:reason
          completionHandler:^(){
    [self shutdownTunnel];
    completionHandler();
  }];
}

- (NSError*)setTunnelAddress:(nw_endpoint_t)endpoint {
  if (nw_endpoint_get_type(endpoint) != nw_endpoint_type_address) {
    return vpnPosixError(EINVAL, @"failed to set tunnel address");
  }
  const struct sockaddr *sa = nw_endpoint_get_address(endpoint);

  NSError *err = nil;
  int sock = socket(sa->sa_family, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    return vpnPosixError(errno, @"failed to set tunnel address");
  }

  if (sa->sa_family == AF_INET) {
    struct ifaliasreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifra_name, nw_interface_get_name(self.virtualInterface), IFNAMSIZ);
    memcpy(&ifr.ifra_addr, sa, sa->sa_len);
    
    // Set the netmask to 255.255.255.255
    struct sockaddr_in *mask = (struct sockaddr_in*)&ifr.ifra_mask;
    memset(mask, 0, sizeof(struct sockaddr_in));
    mask->sin_family = AF_INET;
    mask->sin_len = sizeof(struct sockaddr_in);
    mask->sin_addr.s_addr = 0xffffffff;

    // Do we really need to set a broadcast address?
    struct sockaddr_in *bcast = (struct sockaddr_in*)&ifr.ifra_broadaddr;
    memset(bcast, 0, sizeof(struct sockaddr_in));
    bcast->sin_family = AF_INET;
    bcast->sin_len = sizeof(struct sockaddr_in);
    bcast->sin_addr.s_addr = 0xffffffff;

    if (ioctl(sock, SIOCAIFADDR, &ifr) < 0) {
      err = vpnPosixError(errno, @"failed to set tunnel address");
    } else {
      self.ipv4address = endpoint;
    }
  } else if (sa->sa_family == AF_INET6) {
    struct in6_aliasreq ifr6;
    memset(&ifr6, 0, sizeof(ifr6));
    strncpy(ifr6.ifra_name, nw_interface_get_name(self.virtualInterface), IFNAMSIZ);
    memcpy(&ifr6.ifra_addr, sa, sa->sa_len);
    ifr6.ifra_lifetime.ia6t_vltime = 0xffffffff;
    ifr6.ifra_lifetime.ia6t_pltime = 0xffffffff;

    // Set the prefix length to 128
    struct sockaddr_in6 *mask = (struct sockaddr_in6*)&ifr6.ifra_prefixmask;
    mask->sin6_family = AF_INET6;
    mask->sin6_len = sizeof(struct sockaddr_in6);
    memset(&mask->sin6_addr, 0xff, sizeof(struct in6_addr));

    if (ioctl(sock, SIOCAIFADDR_IN6, &ifr6) < 0) {
      err = vpnPosixError(errno, @"failed to set tunnel address");
    } else {
      self.ipv6address = endpoint;
    }
  } else {
    // We don't recognize this address type.
    err = vpnPosixError(EAFNOSUPPORT, @"failed to set tunnel address");
  }
  close(sock);
  return err;
}

- (void) tunnelWorker:(id)arg {
  size_t mtu = self.mtu;
  uint8_t plaintext[mtu + WG_PACKET_ALIGN];
  uint32_t header;

  NSThread* thread = [NSThread currentThread];
  while (!thread.cancelled) {
    NSMutableData* buffer = [NSMutableData dataWithLength:mtu + WG_PACKET_ALIGN];

    struct iovec iov[2];
    iov[0].iov_base = &header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_base = buffer.mutableBytes;
    iov[1].iov_len = mtu;

    int rx = readv(m_tunfd, iov, 2);
    if (rx == 0) {
      // Socket has closed.
      NSLog(@"utun closed");
      return;
    }
    if (rx < 0) {
      // Socket error occurred.
      NSLog(@"utun error: %s", strerror(errno));
      if (errno == EINTR) continue;
      return;
    }
    if ((rx < sizeof(header)) || (rx > (mtu - sizeof(header)))) {
      continue;
    }
    buffer.length = rx - sizeof(header);

    // I think there is a small bug in boringtun to do with message padding.
    // The wireguard protocol states that the encapsulated packet must first
    // be padded out to a multiple of 16 bytes in length, but boringtun does
    // no such padding during encryption. So let's do it manually ourself.
    int tail = buffer.length % WG_PACKET_ALIGN;
    if (tail) {
      buffer.length += WG_PACKET_ALIGN - tail;
    }

    // If there is no peer to handle this packet, then drop it.
    if (!self.peer) {
      continue;
    }

    [self.peer writePacket:htonl(header)
                  withData:buffer];
  }
}

- (void)shutdownTunnel {
  self.virtualInterface = nil;

  // Shutdown the tunnel worker.
  if (m_worker) {
    shutdown(m_tunfd, SHUT_RDWR);
    [m_worker cancel];
    m_worker = nil;

    dispatch_time_t delay = dispatch_time(DISPATCH_TIME_NOW, WG_WORKQUEUE_TIMEOUT);
    dispatch_semaphore_wait(m_semaphore, delay);
  }

  if (m_tunfd >= 0) {
    close(m_tunfd);
    m_tunfd = -1;
  }

  if (m_rtsock >= 0) {
    close(m_rtsock);
    m_rtsock = -1;
  }
}

- (void)cancelTunnelWithError:(NSError*)error {
  if (self.peer) {
    [self.peer cancelWithError:error];
    self.peer = nil;
  }

  [self shutdownTunnel];
}

- (WireguardStatus*)getStatus {
  if (!self.peer) {
    return [WireguardStatus new];
  }
  WireguardStatus* result = self.peer.status;

  // Get the interface addresses.
  if (self.ipv4address) {
    char* addrstr = nw_endpoint_copy_address_string(self.ipv4address);
    result.ipv4address = [[NSString alloc] initWithBytesNoCopy:addrstr
                                                        length:strlen(addrstr)
                                                      encoding:NSASCIIStringEncoding
                                                  freeWhenDone:YES];
  }
  if (self.ipv6address) {
    char* addrstr = nw_endpoint_copy_address_string(self.ipv6address);
    result.ipv6address = [[NSString alloc] initWithBytesNoCopy:addrstr
                                                        length:strlen(addrstr)
                                                      encoding:NSASCIIStringEncoding
                                                  freeWhenDone:YES];
  }
  if (m_ipv4gateway) {
    char* addrstr = nw_endpoint_copy_address_string(m_ipv4gateway);
    result.ipv4gateway = [[NSString alloc] initWithBytesNoCopy:addrstr
                                                        length:strlen(addrstr)
                                                      encoding:NSASCIIStringEncoding
                                                  freeWhenDone:YES];
  }
  if (m_ipv6gateway) {
    char* addrstr = nw_endpoint_copy_address_string(m_ipv6gateway);
    result.ipv6gateway = [[NSString alloc] initWithBytesNoCopy:addrstr
                                                        length:strlen(addrstr)
                                                      encoding:NSASCIIStringEncoding
                                                  freeWhenDone:YES];
  }

  return result;
}

- (void)setMtu:(NSUInteger)mtu {
  _mtu = mtu;

  struct ifreq ifr;
  strncpy(ifr.ifr_name, nw_interface_get_name(self.virtualInterface), sizeof(ifr.ifr_name));
  ifr.ifr_mtu = _mtu;
  if (ioctl(m_tunfd, SIOCSIFMTU, &ifr) != 0) {
    NSLog(@"mtu update failed: %s", strerror(errno));
  }
}

static void rtmAppendAddr(struct rt_msghdr* rtm, size_t maxlen, int rtaddr, const void* sa) {
  size_t sa_len = ((const struct sockaddr*)sa)->sa_len;
  if ((rtm->rtm_addrs & rtaddr) != 0) {
    return;
  }
  if ((rtm->rtm_msglen + sa_len) > maxlen) {
    return;
  }

  memcpy((char*)rtm + rtm->rtm_msglen, sa, sa_len);
  rtm->rtm_addrs |= rtaddr;
  rtm->rtm_msglen += sa_len;
  if (rtm->rtm_msglen % sizeof(uint32_t)) {
    rtm->rtm_msglen += sizeof(uint32_t) - (rtm->rtm_msglen % sizeof(uint32_t));
  }
}

- (void)rtmSendRoute:(int)action
       toDestination:(const struct sockaddr*)dest
          withPrefix:(NSUInteger)plen
            andFlags:(int)flags {
  constexpr size_t rtm_max_size = sizeof(struct rt_msghdr) +
                                  sizeof(struct sockaddr_in6) * 2 +
                                  sizeof(struct sockaddr_dl);
  char buf[rtm_max_size] = {0};
  struct rt_msghdr* rtm = (struct rt_msghdr*)buf;

  rtm->rtm_msglen = sizeof(struct rt_msghdr);
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = action;
  rtm->rtm_index = nw_interface_get_index(self.virtualInterface);
  rtm->rtm_flags = flags | RTF_STATIC | RTF_UP;
  rtm->rtm_addrs = 0;
  rtm->rtm_pid = 0;
  rtm->rtm_seq = m_rtseq++;
  rtm->rtm_errno = 0;
  rtm->rtm_inits = 0;
  memset(&rtm->rtm_rmx, 0, sizeof(rtm->rtm_rmx));

  // Append RTA_DST
  rtmAppendAddr(rtm, rtm_max_size, RTA_DST, dest);

  // Append RTA_GATEWAY - unless deleting the route.
  if (action != RTM_DELETE) {
    const char* ifname = nw_interface_get_name(self.virtualInterface);
    struct sockaddr_dl datalink;
    memset(&datalink, 0, sizeof(datalink));
    datalink.sdl_family = AF_LINK;
    datalink.sdl_len = offsetof(struct sockaddr_dl, sdl_data) + strlen(ifname);
    datalink.sdl_index = nw_interface_get_index(self.virtualInterface);
    datalink.sdl_type = IFT_OTHER;
    datalink.sdl_nlen = strlen(ifname);
    datalink.sdl_alen = 0;
    datalink.sdl_slen = 0;
    memcpy(datalink.sdl_data, ifname, datalink.sdl_nlen);
    rtmAppendAddr(rtm, rtm_max_size, RTA_GATEWAY, (struct sockaddr*)&datalink);
  }

  // Append RTM_NETMASK
  if (dest->sa_family == AF_INET6) {
    struct sockaddr_in6 mask;
    memset(&mask, 0, sizeof(mask));
    mask.sin6_family = AF_INET6;
    mask.sin6_len = sizeof(mask);
    memset(&mask.sin6_addr.s6_addr, 0xff, plen / 8);
    if (plen % 8) {
      mask.sin6_addr.s6_addr[plen / 8] = 0xff ^ (0xff >> (plen % 8));
    }
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &mask);
  } else if (dest->sa_family == AF_INET) {
    struct sockaddr_in mask;
    memset(&mask, 0, sizeof(mask));
    mask.sin_family = AF_INET;
    mask.sin_len = sizeof(mask);
    mask.sin_addr.s_addr = 0xffffffff;
    if (plen < 32) {
      mask.sin_addr.s_addr ^= htonl(0xffffffff >> plen);
    }
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &mask);
  }

  // Send the routing message into the kernel.
  int len = write(m_rtsock, rtm, rtm->rtm_msglen);
  if (len == rtm->rtm_msglen) {
    return;
  }
  if ((action == RTM_ADD) && (errno == EEXIST)) {
    return;
  }
  if ((action == RTM_DELETE) && (errno == ESRCH)) {
    return;
  }
  NSLog(@"Failed to send route to kernel: %s", strerror(errno));
}

@end
