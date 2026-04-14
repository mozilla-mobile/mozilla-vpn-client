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
#include <sys/sysctl.h>

// Private method in the network framework
extern "C" nw_interface_t nw_interface_create_with_index(int ifindex);

@implementation RouteManager {
  // The routing socket.
  CFSocketNativeHandle m_sockfd;
  CFSocketRef          m_socket;
  CFRunLoopSourceRef   m_source;
  int                  m_rtseq;

  NSObject<RouteManagerDelegate>* m_delegate;
}

static void rawSockCallback(CFSocketRef s, CFSocketCallBackType cbType,
                            CFDataRef address, const void * data, void *info) {
  RouteManager* monitor = (__bridge RouteManager*)info;
  if (cbType == kCFSocketDataCallBack) {
    [monitor rtDataCallback:(__bridge NSData*)data];
  } else {
    NSLog(@"rawSockCallback: unexpected type %d", (int)cbType);
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

static NSString* rtmAddrString(const void *ptr) {
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

static void rtmLogRouteMsg(const struct rt_msghdr* rtm, CFArrayRef addrlist) {
#ifdef MZ_DEBUG
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

  // Figure out the relevant interface name.
  char ifname[IF_NAMESIZE] = "null";
  if (rtm->rtm_addrs & RTA_IFP) {
    const struct sockaddr_dl* sdl = (const struct sockaddr_dl*)rtmLookupAddr(rtm, RTA_IFP, addrlist);
    if (sdl && sdl->sdl_family == AF_LINK) {
      if_indextoname(sdl->sdl_index, ifname);
    }
  } else if (rtm->rtm_index) {
    if_indextoname(rtm->rtm_index, ifname);
  }

  NSMutableString* details = [NSMutableString stringWithCapacity:0];
  // Log relevant updates to the routing table.
  if (addrlist && CFArrayGetCount(addrlist)) {
    [details appendFormat:@" addrs(%x)", rtm->rtm_addrs];
    for (int i = 0; i < CFArrayGetCount(addrlist); i++) {
      [details appendString:@" "];
      [details appendString:rtmAddrString(CFArrayGetValueAtIndex(addrlist, i))];
    }
  }

  NSLog(@"route %@ %s:%@", rtmType, ifname, details);
#endif
}

// Compare memory against zero.
static int memcmpzero(const void* data, size_t len) {
  const uint8_t* ptr = static_cast<const uint8_t*>(data);
  while (len--) {
    if (*ptr++) return 1;
  }
  return 0;
}

- (id)init {
  self = [super init];
  NSLog(@"route manager created");

  m_rtseq = 0;
  m_sockfd = socket(PF_ROUTE, SOCK_RAW, 0);
  if (m_sockfd < 0) {
    NSLog(@"failed to create routing socket: %s", strerror(errno));
  }
  CFSocketContext ctx = { .info = (__bridge void *)self };
  m_socket = CFSocketCreateWithNative(kCFAllocatorDefault, m_sockfd, kCFSocketDataCallBack,
                                      rawSockCallback, &ctx);

  // Create a source and attach it to the main run loop.
  m_source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, m_socket, 0);
  CFRunLoopAddSource(CFRunLoopGetMain(), m_source, kCFRunLoopDefaultMode);

  return self;
}

- (void)dealloc {
  NSLog(@"route manager destroyed");

  CFSocketInvalidate(m_socket);
  CFRelease(m_socket);
  close(m_sockfd);

  CFRunLoopRemoveSource(CFRunLoopGetMain(), m_source, kCFRunLoopDefaultMode);
  CFRelease(m_source);

#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

- (void)startWithDelegate:(NSObject<RouteManagerDelegate>*)delegate {
  m_delegate = delegate;

  // Grab the default routes at startup.
  NSLog(@"Fetching default routes");
  [self rtmFetchRoutes:AF_INET];
  [self rtmFetchRoutes:AF_INET6];
}

- (void)rtDataCallback:(NSData*)data {
#ifndef RTMSG_NEXT
#  define RTMSG_NEXT(_rtm_) \
    (struct rt_msghdr*)((char*)(_rtm_) + (_rtm_)->rtm_msglen)
#endif

  const char* buf = (const char*)data.bytes;
  const struct rt_msghdr* rtm = (const struct rt_msghdr*)(buf);
  const struct rt_msghdr* end = (const struct rt_msghdr*)(buf + data.length);
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

  // Log the relevant routing messages.
  rtmLogRouteMsg(rtm, addrlist);

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

  // Notify the delegates about the default route update.
  const struct sockaddr* gateway = rtmLookupAddr(rtm, RTA_GATEWAY, addrlist);
  const struct sockaddr* dst = rtmLookupAddr(rtm, RTA_DST, addrlist);
  nw_interface_t iface = nw_interface_create_with_index(ifindex);
  if (m_delegate) {
    NSData* gwData = [NSData dataWithBytes:gateway
                                    length:gateway->sa_len];
    [m_delegate defaultRouteChanged:dst->sa_family
                       viaInterface:iface
                        withGateway:gwData];
  }
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

  // Log the relevant routing messages.
  rtmLogRouteMsg(rtm, addrlist);

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

  // Delete exclusion routes.
  const struct sockaddr* dst = rtmLookupAddr(rtm, RTA_DST, addrlist);
  NSLog(@"Lost default route");
  if (m_delegate) {
    [m_delegate defaultRouteChanged:dst->sa_family
                       viaInterface:nil
                        withGateway:nil];
  }
}

- (void)rtmFetchRoutes:(int)family {
  int mib[] = { CTL_NET, PF_ROUTE, 0, family, NET_RT_DUMP, 0 };
  int miblen = sizeof(mib)/sizeof(int);
  size_t bufsize = 0;

  // Get the size of the routing table.
  if (sysctl(mib, miblen, nullptr, &bufsize, nullptr, 0) < 0) {
    NSLog(@"Failed to get routing table size: %s", strerror(errno));
    return;
  }
  // Add a little extra in case of a race condition.
  bufsize += 4096;

  // Fetch a copy of the routing table from the kernel.
  char* buffer = (char*)malloc(bufsize);
  if (sysctl(mib, miblen, buffer, &bufsize, nullptr, 0) < 0) {
    NSLog(@"Failed to feetch routing table: %s", strerror(errno));
    free(buffer);
    return;
  }

  [self rtDataCallback:[NSData dataWithBytesNoCopy:buffer
                                     length:bufsize
                               freeWhenDone:true]];
}

- (void) rtmSendRoute:(int)action
        toDestination:(NSData*)dst
           withPrefix:(unsigned int)plen
         viaInterface:(unsigned int)ifindex
          withGateway:(NSData*)gateway
             andFlags:(int)flags {
  constexpr size_t rtm_max_size = sizeof(struct rt_msghdr) +
                                  sizeof(struct sockaddr_in6) * 2 +
                                  sizeof(struct sockaddr_storage);
  char buf[rtm_max_size] = {0};
  struct rt_msghdr* rtm = reinterpret_cast<struct rt_msghdr*>(buf);

  rtm->rtm_msglen = sizeof(struct rt_msghdr);
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = action;
  rtm->rtm_index = ifindex;
  rtm->rtm_flags = flags | RTF_STATIC | RTF_UP;
  rtm->rtm_addrs = 0;
  rtm->rtm_pid = 0;
  rtm->rtm_seq = m_rtseq++;
  rtm->rtm_errno = 0;
  rtm->rtm_inits = 0;
  memset(&rtm->rtm_rmx, 0, sizeof(rtm->rtm_rmx));

  // Append RTA_DST
  const struct sockaddr* dstAddr = (const struct sockaddr*)dst.bytes;
  rtmAppendAddr(rtm, rtm_max_size, RTA_DST, dstAddr);

  // Append RTA_GATEWAY
  if (gateway != nullptr) {
    const struct sockaddr* gw = (const struct sockaddr*)gateway.bytes;
    if ((gw->sa_family == AF_INET) || (gw->sa_family == AF_INET6)) {
      rtm->rtm_flags |= RTF_GATEWAY;
    }
    rtmAppendAddr(rtm, rtm_max_size, RTA_GATEWAY, gw);
  }

  // Append RTA_NETMASK
  if (dstAddr->sa_family == AF_INET6) {
    struct sockaddr_in6 mask;
    memset(&mask, 0, sizeof(mask));
    mask.sin6_family = AF_INET6;
    mask.sin6_len = sizeof(mask);
    memset(&mask.sin6_addr.s6_addr, 0xff, plen / 8);
    if (plen % 8) {
      mask.sin6_addr.s6_addr[plen / 8] = 0xFF ^ (0xFF >> (plen % 8));
    }
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &mask);
  } else if (dstAddr->sa_family == AF_INET) {
    struct sockaddr_in mask;
    memset(&mask, 0, sizeof(mask));
    mask.sin_family = AF_INET;
    mask.sin_len = sizeof(struct sockaddr_in);
    mask.sin_addr.s_addr = 0xffffffff;
    if (plen < 32) {
      mask.sin_addr.s_addr ^= htonl(0xffffffff >> plen);
    }
    rtmAppendAddr(rtm, rtm_max_size, RTA_NETMASK, &mask);
  }

  // Send the routing message into the kernel.
  CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)rtm, rtm->rtm_msglen);
  CFSocketError err = CFSocketSendData(m_socket, NULL, data, 1);
  CFRelease(data);
  if (err != kCFSocketSuccess) {
    NSLog(@"Failed to send route to kernel: %d", (int)err);
    return;
  }
}

@end
