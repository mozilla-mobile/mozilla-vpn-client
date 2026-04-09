/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "routemanager.h"

#import <CoreFoundation/CoreFoundation.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

@implementation RouteManager {
  // The routing socket.
  CFSocketNativeHandle m_sockfd;
  CFSocketRef          m_socket;
  CFRunLoopSourceRef   m_source;
  int                  m_rtseq;

  // The default route.
  int                     m_defaultIfindexIpv4;
  struct sockaddr_storage m_defaultGatewayIpv4;
  int                     m_defaultIfindexIpv6;
  struct sockaddr_storage m_defaultGatewayIpv6;
}

static void rawSockCallback(CFSocketRef s, CFSocketCallBackType cbType, CFDataRef address, const void * data, void *info) {
  RouteManager* monitor = (RouteManager*)info;
  if (cbType == kCFSocketDataCallBack) {
    [monitor rtDataCallback:(CFDataRef)data];
  } else {
    NSLog(@"rawSockCallback: unexpected type %d", (int)cbType);
  }
}

static void rtmAppendAddr(struct rt_msghdr* rtm, size_t maxlen, int rtaddr, void* sa) {
  size_t sa_len = static_cast<const struct sockaddr*>(sa)->sa_len;
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

static CFArrayRef rtmParseAddrList(const struct rt_msghdr* rtm, size_t hdrlen) {
  constexpr int minlen = offsetof(struct sockaddr, sa_len) + sizeof(u_short);
  const UInt8* data = (const UInt8*)rtm;
  CFMutableArrayRef list = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
  size_t offset = hdrlen;

  while ((offset + minlen) <= rtm->rtm_msglen) {
    struct sockaddr* sa = (struct sockaddr*)(data + offset);
    int paddedSize = sa->sa_len;
    if (!paddedSize || (paddedSize % sizeof(uint32_t))) {
      paddedSize += sizeof(uint32_t) - (paddedSize % sizeof(uint32_t));
    }
    if ((offset + paddedSize) > rtm->rtm_msglen) {
      break;
    }
    CFDataRef saData = CFDataCreate(kCFAllocatorDefault, (UInt8*)sa, paddedSize);
    CFArrayAppendValue(list, saData);
    offset += paddedSize;
  }
  return list;
}

static const struct sockaddr* rtmLookupAddr(const struct rt_msghdr* rtm, int which, CFArrayRef addrs) {
  if ((rtm->rtm_addrs & which) == 0) {
    // Address is not included in this message
    return NULL;
  }
  
  // Figure out the index at which it should be present.
  int index = 0;
  for (int mask = 1; (mask & which) == 0; mask <<= 1) {
    if (rtm->rtm_addrs & mask) {
      index++;
    }
  }

  // Return the address in question.
  if (index >= CFArrayGetCount(addrs)) {
    return NULL;
  }
  CFDataRef data = (CFDataRef)CFArrayGetValueAtIndex(addrs, index);
  return (struct sockaddr*)CFDataGetBytePtr(data);
}

NSString* rtmAddrString(const void *ptr) {
  CFDataRef data = (CFDataRef)ptr;
  const struct sockaddr* sa = (const struct sockaddr*)CFDataGetBytePtr(data);
  if (sa->sa_len > CFDataGetLength(data)) {
    return @"truncated";
  }

  if (sa->sa_family == AF_INET) {
    const struct sockaddr_in* sin = (const struct sockaddr_in*)sa;
    return [NSString stringWithUTF8String:inet_ntoa(sin->sin_addr)];
  } else if (sa->sa_family == AF_INET6) {
    const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)sa;
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof(buf));
    return [NSString stringWithUTF8String:buf];
  } else if (sa->sa_family == AF_LINK) {
    const struct sockaddr_dl* sdl = (const struct sockaddr_dl*)sa;
    return [NSString stringWithFormat:@"link#%d:%s", sdl->sdl_index, link_ntoa(sdl)];
  } else if (sa->sa_family == AF_UNSPEC) {
    return @"unspec";
  }

  return [NSString stringWithFormat:@"unknown(af=%d)", sa->sa_family];
}

void rtmLogMsg(struct rt_msghdr* rtm, CFArrayRef addrlist) {
  NSString* rtmType = nullptr;
  switch (rtm->rtm_type) {
    case RTM_ADD:
      rtmType = @"RTM_ADD";
      break;
    case RTM_DELETE:
      rtmType = @"RTM_DELETE";
      break;
    case RTM_CHANGE:
      rtmType = @"RTM_CHANGE";
      break;
    case RTM_GET:
      rtmType = @"RTM_GET";
      break;
    case RTM_IFINFO:
      rtmType = @"RTM_IFINFO";
      break;
    default:
      rtmType = [NSString stringWithFormat:@"unknown(%d)", rtm->rtm_type];
      break;
  }

#if 0
  // Figure out the relevant interface name.
  char ifname[IF_NAMESIZE] = "null";
  if (rtm->rtm_addrs & RTA_IFP) {
    const struct sockaddr_dl* sdl = (const struct sockaddr_dl*)rtmLookupAddr(rtm, RTA_IFP, addrlist);
    if (sdl && sdl->sdl_family == AF_LINK) {
      if_indextoname( sdl->sdl_index, ifname);
    }
  } else if (rtm->rtm_ifindex) {
    if_indextoname(rtm->rtm_ifindex, ifname);
  }
#endif

  // Log relevant updates to the routing table.
  NSMutableString* list = [NSMutableString stringWithCapacity:0];
  if (addrlist) {
    for (int i = 0; i < CFArrayGetCount(addrlist); i++) {
      if (list.length > 0) {
        [list appendString:@" "];
      }
      [list appendString:rtmAddrString(CFArrayGetValueAtIndex(addrlist, i))];
    }
  }

  NSLog(@"route %@: addrs(%x) %@", rtmType, rtm->rtm_addrs, list);
}

// Compare memory against zero.
static int memcmpzero(const void* data, size_t len) {
  const uint8_t* ptr = static_cast<const uint8_t*>(data);
  while (len--) {
    if (*ptr++) return 1;
  }
  return 0;
}

- (id)initWithRunLoop:(CFRunLoopRef)runloop {
  self = [super init];

  m_rtseq = 0;
  m_sockfd = socket(PF_ROUTE, SOCK_RAW, 0);
  if (m_sockfd < 0) {
    NSLog(@"failed to create routing socket: %s", strerror(errno));
  }
  CFSocketContext ctx = { .info = self };
  m_socket = CFSocketCreateWithNative(kCFAllocatorDefault, m_sockfd, kCFSocketDataCallBack,
                                      rawSockCallback, &ctx);

  m_source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, m_socket, 100);
  CFRunLoopAddSource(runloop, m_source, kCFRunLoopCommonModes);

  // Grab the default routes at startup.
  NSLog(@"Fetching default routes");
  [self rtmFetchRoutes:AF_INET];
  [self rtmFetchRoutes:AF_INET6];

  return self;
}

- (void)dealloc {
  CFRelease(m_socket);
  close(m_sockfd);
  [super dealloc];
}

- (void)rtDataCallback:(CFDataRef)data {
#ifndef RTMSG_NEXT
#  define RTMSG_NEXT(_rtm_) \
    (struct rt_msghdr*)((char*)(_rtm_) + (_rtm_)->rtm_msglen)
#endif

  char* buf = (char*)CFDataGetBytePtr(data);
  long len = CFDataGetLength(data);
  struct rt_msghdr* rtm = reinterpret_cast<struct rt_msghdr*>(buf);
  struct rt_msghdr* end = reinterpret_cast<struct rt_msghdr*>(buf + len);
  while (rtm < end) {
    // Ensure the message fits within the buffer
    if (RTMSG_NEXT(rtm) > end) {
      NSLog(@"Routing message overflowed with length: %d", rtm->rtm_msglen);
      break;
    }

    CFArrayRef addrs = NULL;
    switch (rtm->rtm_type) {
      case RTM_ADD:
        addrs = rtmParseAddrList(rtm, sizeof(struct rt_msghdr));
        [self handleRtmUpdate:rtm withAddrList:addrs];
        break;
      case RTM_DELETE:
        addrs = rtmParseAddrList(rtm, sizeof(struct rt_msghdr));
        [self handleRtmDelete:rtm withAddrList:addrs];
        break;
      case RTM_CHANGE:
        addrs = rtmParseAddrList(rtm, sizeof(struct rt_msghdr));
        [self handleRtmUpdate:rtm withAddrList:addrs];
        break;
      case RTM_GET:
        addrs = rtmParseAddrList(rtm, sizeof(struct rt_msghdr));
        [self handleRtmUpdate:rtm withAddrList:addrs];
        break;
      case RTM_IFINFO:
        //message.remove(0, sizeof(struct if_msghdr));
        //handleIfaceInfo((struct if_msghdr*)rtm, message);
        break;
      default:
        break;
    }

    rtmLogMsg(rtm, addrs);
    if (addrs) {
      CFRelease(addrs);
    }

    rtm = RTMSG_NEXT(rtm);
  }
}

- (void) handleRtmUpdate:(const struct rt_msghdr*)rtm
            withAddrList:(CFArrayRef)addrlist {
  int ifindex = rtm->rtm_index;

  // We expect all useful routes to contain a destination, netmask and gateway.
  if (!(rtm->rtm_addrs & RTA_DST) || !(rtm->rtm_addrs & RTA_GATEWAY) ||
      !(rtm->rtm_addrs & RTA_NETMASK) || (CFArrayGetCount(addrlist) < 3)) {
    return;
  }
  // Ignore interface-scoped routes, we want to find the default route to the
  // internet in the global scope.
  if (rtm->rtm_flags & RTF_IFSCOPE) {
    return;
  }
  // Ignore route changes that we caused, or routes on the tunnel interface.
  //if (rtm->rtm_index == m_ifindex) {
  //  return;
  //}
  if ((rtm->rtm_pid == getpid()) && (rtm->rtm_type != RTM_GET)) {
    return;
  }

  // Special case: If RTA_IFP is set, then we should get the interface index
  // from the address list instead of rtm_index.
  if (rtmLookupAddr(rtm, RTA_IFP, addrlist)) {
    const struct sockaddr_dl* sdl = (const struct sockaddr_dl*)rtmLookupAddr(rtm, RTA_IFP, addrlist);
    if (sdl && sdl->sdl_family == AF_LINK) {
      ifindex = sdl->sdl_index;
    }
  }

  // Check for a default route, which should have a netmask of zero.
  const struct sockaddr* mask = rtmLookupAddr(rtm, RTA_NETMASK, addrlist);
  if (mask->sa_family == AF_INET) {
    struct sockaddr_in sin;
    //Q_ASSERT(mask->sa_len <= sizeof(sin));
    memset(&sin, 0, sizeof(sin));
    memcpy(&sin, mask, mask->sa_len);
    if (memcmpzero(&sin.sin_addr, sizeof(sin.sin_addr)) != 0) {
      return;
    }
  } else if (mask->sa_family == AF_INET6) {
    struct sockaddr_in6 sin6;
    //Q_ASSERT(mask->sa_len <= sizeof(sin6));
    memset(&sin6, 0, sizeof(sin6));
    memcpy(&sin6, mask, mask->sa_len);
    if (memcmpzero(&sin6.sin6_addr, sizeof(sin6.sin6_addr)) != 0) {
      return;
    }
  } else if (mask->sa_family != AF_UNSPEC) {
    // The default route sometimes sets a netmask of AF_UNSPEC.
    return;
  }

  // Determine if this is the IPv4 or IPv6 default route.
  const struct sockaddr* gw = rtmLookupAddr(rtm, RTA_GATEWAY, addrlist);
  const struct sockaddr* dst = rtmLookupAddr(rtm, RTA_DST, addrlist);
  int rtm_type = RTM_ADD;
  int prev_ifindex = 0;
  if (dst->sa_family == AF_INET) {
    prev_ifindex = m_defaultIfindexIpv4;
    if (m_defaultIfindexIpv4 != 0) {
      rtm_type = RTM_CHANGE;
    }
    memcpy(&m_defaultGatewayIpv4, gw, gw->sa_len);
    m_defaultIfindexIpv4 = ifindex;
  } else if (dst->sa_family == AF_INET6) {
    prev_ifindex = m_defaultIfindexIpv6;
    if (m_defaultIfindexIpv6 != 0) {
      rtm_type = RTM_CHANGE;
    }
    memcpy(&m_defaultGatewayIpv6, gw, gw->sa_len);
    m_defaultIfindexIpv6 = ifindex;
  } else {
    return;
  }

#if 0
  // Update the captured default route.
  const IPAddress prefix(QHostAddress(dst), 0);
  if (prev_ifindex == ifindex) {
    rtmSendRoute(RTM_CHANGE, prefix, ifindex, addrlist[1], RTF_IFSCOPE);
  } else {
    if (prev_ifindex != 0) {
      rtmSendRoute(RTM_DELETE, prefix, ifindex, nullptr, RTF_IFSCOPE);
    }
    rtmSendRoute(RTM_ADD, prefix, ifindex, addrlist[1], RTF_IFSCOPE);
  }
#endif
}

- (void) handleRtmDelete:(const struct rt_msghdr*)rtm
            withAddrList:(CFArrayRef)addrlist {

  // Ignore routing changes on the tunnel interface.
  //if (rtm->rtm_index == m_ifindex) {
  //  return;
  //}

  // We expect all useful routes to contain a destination, netmask and gateway.
  if (!(rtm->rtm_addrs & RTA_DST) || !(rtm->rtm_addrs & RTA_GATEWAY) ||
      !(rtm->rtm_addrs & RTA_NETMASK) || (CFArrayGetCount(addrlist) < 3)) {
    return;
  }
  // Ignore interface-scoped routes, we want to find the default route to the
  // internet in the global scope.
  if (rtm->rtm_flags & RTF_IFSCOPE) {
    return;
  }

  // Check for a default route, which should have a netmask of zero.
  const struct sockaddr* mask = rtmLookupAddr(rtm, RTA_NETMASK, addrlist);
  if (mask->sa_family == AF_INET) {
    struct sockaddr_in sin;
    //Q_ASSERT(mask->sa_len <= sizeof(sin));
    memset(&sin, 0, sizeof(sin));
    memcpy(&sin, mask, mask->sa_len);
    if (memcmpzero(&sin.sin_addr, sizeof(sin.sin_addr)) != 0) {
      return;
    }
  } else if (mask->sa_family == AF_INET6) {
    struct sockaddr_in6 sin6;
    //Q_ASSERT(mask->sa_len <= sizeof(sin6));
    memset(&sin6, 0, sizeof(sin6));
    memcpy(&sin6, mask, mask->sa_len);
    if (memcmpzero(&sin6.sin6_addr, sizeof(sin6.sin6_addr)) != 0) {
      return;
    }
  } else if (mask->sa_family != AF_UNSPEC) {
    // We have sometimes seen the default route reported with AF_UNSPEC.
    return;
  }

  // Clear the default gateway
  const struct sockaddr* dst = rtmLookupAddr(rtm, RTA_DST, addrlist);
  if (dst->sa_family == AF_INET) {
    memset(&m_defaultGatewayIpv4, 0, sizeof(m_defaultGatewayIpv4));
    m_defaultIfindexIpv4 = 0;
  } else if (dst->sa_family == AF_INET6) {
    memset(&m_defaultGatewayIpv6, 0, sizeof(m_defaultGatewayIpv6));
    m_defaultIfindexIpv6 = 0;
  }

  // Delete the captured default route.
  //if (m_defaultRouteCapture) {
  //  rtmSendRoute(RTM_DELETE, IPAddress(QHostAddress(dst), 0), rtm->rtm_index,
  //              nullptr, RTF_IFSCOPE);
  //}

  // Delete exclusion routes.
  NSLog(@"Lost default route");
}

- (void)rtmFetchRoutes:(int)family {
  constexpr size_t rtm_max_size =
      sizeof(struct rt_msghdr) + sizeof(struct sockaddr_storage) * 2;
  char buf[rtm_max_size] = {0};
  struct rt_msghdr* rtm = reinterpret_cast<struct rt_msghdr*>(buf);

  rtm->rtm_msglen = sizeof(struct rt_msghdr);
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = RTM_GET;
  rtm->rtm_flags = RTF_UP | RTF_GATEWAY;
  rtm->rtm_addrs = 0;
  rtm->rtm_pid = 0;
  rtm->rtm_seq = m_rtseq++;
  rtm->rtm_errno = 0;
  rtm->rtm_inits = 0;
  memset(&rtm->rtm_rmx, 0, sizeof(rtm->rtm_rmx));

  if (family == AF_INET) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(struct sockaddr_in);
    rtmAppendAddr(rtm, rtm_max_size, RTA_DST, &sin);
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &sin);
  } else if (family == AF_INET6) {
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(struct sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(struct sockaddr_in6);
    rtmAppendAddr(rtm, rtm_max_size, RTA_DST, &sin6);
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &sin6);
  } else {
    NSLog(@"Failed to request routing table: unsupported address family");
    return;
  }

  // Send the routing message into the kernel.
  CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)rtm, rtm->rtm_msglen);
  CFSocketError err = CFSocketSendData(m_socket, NULL, data, 1);
  CFRelease(data);
  if (err != kCFSocketSuccess) {
    NSLog(@"Failed to request routing table: %d", (int)err);
    return;
  }
}

@end
