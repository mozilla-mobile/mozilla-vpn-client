/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "wireguardtunnel.h"

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>

#include <errno.h>
#include <net/if.h>
#include <net/if_utun.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/in_var.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

extern "C" {
#include "wireguard_ffi.h"
};

// Private method in the network framework
extern "C" nw_interface_t nw_interface_create_with_index_and_name(int ifindex, const char *ifname);

@implementation WireguardStats
// Nothing to see here.
@end

@implementation WireguardTunnel {
  struct wireguard_tunnel*  m_wireguard;

  int                  m_tunfd;
  CFSocketRef          m_socket;
  CFRunLoopSourceRef   m_source;
  CFRunLoopTimerRef    m_timer;

  struct timespec      m_lastHandshake;
  struct timespec      m_handshakeTimeout;

  // The completion handler to run on initial handshake or timeout. 
  void (^m_completionHandler)(NSError *error);
}

constexpr const int WG_PACKET_OVERHEAD = 32;
constexpr const int WG_MTU_OVERHEAD = 80;
constexpr const int WG_MAX_HANDSHAKE_SIZE = 148;
constexpr const int WG_MAX_HANDSHAKE_TIMEOUT = 15;

static void wgLog(const char* msg) {
  NSLog(@"wg: %s", msg);
}

static void utunSockCallback(CFSocketRef s, CFSocketCallBackType cbType,
                             CFDataRef address, const void * data, void *info) {
  WireguardTunnel* tunnel = (__bridge WireguardTunnel*)info;
  NSData* rawData = (__bridge NSData*)data;
  if (cbType != kCFSocketDataCallBack) {
    NSLog(@"utunSockCallback: unexpected type %d", (int)cbType);
  } else if (rawData.length < 4) {
    NSLog(@"utunSockCallback: packet truncated");
  } else {
    NSData* packet = [rawData subdataWithRange:NSMakeRange(4, rawData.length - 4)];
    [tunnel handleOutbound:packet
              withProtocol:htonl(*(uint32_t*)rawData.bytes)];
  }
}

static void wgTimerCallback(CFRunLoopTimerRef t, void *info) {
  WireguardTunnel* tunnel = (__bridge WireguardTunnel*)info;
  [tunnel handleTimer];
}

- (id)init {
  self = [super init];
  set_logging_function(wgLog);

  m_tunfd = -1;
  m_wireguard = nil;

  _mtu = IPV6_MMTU;
  return self;
}

static NSError* errorFromErrno(int code, NSString* desc) {
  if (!desc) {
    desc = @"error occurred";
  }
  NSString* msg = [NSString stringWithFormat:@"%@: %s", desc, strerror(code)];
  NSLog(@"%@", msg);

  return [NSError errorWithDomain:NSPOSIXErrorDomain
                             code:code
                         userInfo:@{NSLocalizedDescriptionKey: msg}];
}

- (void) startTunnelWithOptions:(InterfaceConfig *)options 
              completionHandler:(void (^)(NSError *error)) completionHandler {
  m_completionHandler = completionHandler;
  m_tunfd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
  if (m_tunfd < 0) {
    [self shutdownTunnel:@"tunnel creation failed" withErrno:errno];
    return;
  }

  // Connect to the utun control kernel service.
  struct ctl_info info = {.ctl_name = "com.apple.net.utun_control"};
  int err = ioctl(m_tunfd, CTLIOCGINFO, &info);
  if (err < 0) {
    [self shutdownTunnel:@"kernel utun lookup failed" withErrno:errno];
    return;
  }
  struct sockaddr_ctl addr = {};
  addr.sc_len = sizeof(addr);
  addr.sc_family = AF_SYSTEM;
  addr.ss_sysaddr = AF_SYS_CONTROL;
  addr.sc_id = info.ctl_id;
  addr.sc_unit = 0;
  err = connect(m_tunfd, (struct sockaddr*)&addr, sizeof(addr));
  if (err < 0) {
    [self shutdownTunnel:@"kernel utun connect failed" withErrno:errno];
    return;
  }

  // Get the tunnel device's name.
  struct ifreq ifr;
  socklen_t ifnamesize = sizeof(ifr.ifr_name);
  err = getsockopt(m_tunfd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifr.ifr_name,
                   &ifnamesize);
  if (err < 0) {
    [self shutdownTunnel:@"utun name loookup failed" withErrno:errno];
    return;
  }
  int ifindex = if_nametoindex(ifr.ifr_name);
  self.virtualInterface = nw_interface_create_with_index_and_name(ifindex, ifr.ifr_name);

  // Assign addresses
  if (NSError *err = [self setTunnelAddress:options.deviceIpv4Addr]) {
    [self shutdownTunnel:err];
    return;
  }
  if (NSError *err = [self setTunnelAddress:options.deviceIpv6Addr]) {
    [self shutdownTunnel:err];
    return;
  }

  // Set a base MTU, it will get updated later.
  ifr.ifr_mtu = self.mtu;
  if (ioctl(m_tunfd, SIOCSIFMTU, &ifr) != 0) {
    [self shutdownTunnel:@"failed to set mtu" withErrno:errno];
    return;
  }

  // Bring the device up.
  err = ioctl(m_tunfd, SIOCGIFFLAGS, &ifr);
  if (err != 0) {
    [self shutdownTunnel:@"failed to get interface flags" withErrno:errno];
    return;
  }
  ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  err = ioctl(m_tunfd, SIOCSIFFLAGS, &ifr);
  if (err != 0) {
    [self shutdownTunnel:@"failed to set device up" withErrno:errno];
    return;
  }

  // Wrap the tunnel device in a CFSocket
  CFSocketContext ctx = { .info = (__bridge void *)self };
  m_socket = CFSocketCreateWithNative(kCFAllocatorDefault, m_tunfd,
                                      kCFSocketDataCallBack,
                                      utunSockCallback, &ctx);

  // Create a source and attach it to the main run loop.
  m_source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, m_socket, 0);
  CFRunLoopAddSource(CFRunLoopGetMain(), m_source, kCFRunLoopDefaultMode);

  // (Re)-create the Wireguard tunnel structure.
  if (m_wireguard) {
    tunnel_free(m_wireguard);
  }
  uint32_t index;
  getentropy(&index, sizeof(index));

  char *addrstr = nw_endpoint_copy_address_string(options.serverIpv4Addr);
  NSLog(@"wireguard peer: %s port=%d", addrstr, nw_endpoint_get_port(options.serverIpv4Addr));
  free(addrstr);

  nw_parameters_t params = nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
  self.connection = nw_connection_create(options.serverIpv4Addr, params);
  nw_connection_set_queue(self.connection, dispatch_get_main_queue());

  // The tunnel username should contain the base64-encoded server public key and
  // the password should hold the device private key.
  m_wireguard = new_tunnel(options.privateKey.UTF8String,
                           options.serverPublicKey.UTF8String,
                           nil, // Preshared key
                           300, // Keepalive period
                           index % (1U << 24));

  m_completionHandler = completionHandler;
  nw_connection_set_state_changed_handler(self.connection,
                                          ^(nw_connection_state_t state, nw_error_t err) {
    if (err) {
      CFErrorRef cfError = nw_error_copy_cf_error(err);
      NSLog(@"vpn socket error: %@", (__bridge NSError*)cfError);
      [self shutdownTunnel:(__bridge NSError*)cfError];
      CFRelease(cfError);
    } else if (state == nw_connection_state_cancelled || state == nw_connection_state_failed) {
      NSLog(@"vpn socket closed");
      [self shutdownTunnel:nil];
    } else if (state != nw_connection_state_ready) {
      NSLog(@"vpn socket state %d", state);
    } else {
      NSLog(@"vpn socket opened");
      [self renegotiate];
      [self handleInbound];

      // Start the timer.
      CFRunLoopTimerContext timerContext = { .info = (__bridge void *)self };
      m_timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 0, 0.1, 0, 0,
                                     wgTimerCallback, &timerContext);
      CFRunLoopAddTimer(CFRunLoopGetMain(), m_timer, kCFRunLoopDefaultMode);
    }
  });
  nw_connection_start(self.connection);
}

- (void) stopTunnelWithReason:(NEProviderStopReason)reason 
            completionHandler:(void (^)()) completionHandler {
  m_completionHandler = ^(NSError *error){
    completionHandler();
  };

  [self shutdownTunnel:nil];
}

- (NSError*)setTunnelAddress:(nw_endpoint_t)endpoint {
  if (nw_endpoint_get_type(endpoint) != nw_endpoint_type_address) {
    return errorFromErrno(EINVAL, @"failed to set tunnel address");
  }
  const struct sockaddr *sa = nw_endpoint_get_address(endpoint);

  NSError *err = nil;
  int sock = socket(sa->sa_family, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    return errorFromErrno(errno, @"failed to set tunnel address");
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
      err = errorFromErrno(errno, @"failed to set tunnel address");
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
      err = errorFromErrno(errno, @"failed to set tunnel address");
    }
  } else {
    // We don't recognize this address type.
    err = errorFromErrno(EAFNOSUPPORT, @"failed to set tunnel address");
  }
  close(sock);
  return err;
}

- (void) renegotiate {
  NSLog(@"wireguard renegotiate");
  UInt8* handshake = (UInt8 *)malloc(WG_MAX_HANDSHAKE_SIZE);

  struct wireguard_result r;
  r = wireguard_force_handshake(m_wireguard, handshake, WG_MAX_HANDSHAKE_SIZE);

  // Set a timeout for the handshake to finish.
  clock_gettime(CLOCK_MONOTONIC, &m_handshakeTimeout);
  m_handshakeTimeout.tv_sec += WG_MAX_HANDSHAKE_TIMEOUT;

  dispatch_data_t data = dispatch_data_create(handshake, r.size,
                                              dispatch_get_main_queue(),
                                              DISPATCH_DATA_DESTRUCTOR_FREE);
  [self handleWireguard:(int)r.op withData:data];
}

- (void) handleTimer {
  // Check for a handshake timeout.
  if (m_handshakeTimeout.tv_sec && m_handshakeTimeout.tv_nsec) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (now.tv_sec > m_handshakeTimeout.tv_sec) {
      [self shutdownTunnel:@"handshake timeout" withErrno:ETIMEDOUT];
      return;
    } else if (now.tv_sec < m_handshakeTimeout.tv_sec) {
      // It has not timed out.
    } else if (now.tv_nsec > m_handshakeTimeout.tv_nsec) {
      [self shutdownTunnel:@"handshake timeout" withErrno:ETIMEDOUT];
      return;
    }
  }

  UInt8* handshake = (UInt8 *)malloc(WG_MAX_HANDSHAKE_SIZE);
  struct wireguard_result r;
  r = wireguard_tick(m_wireguard, handshake, WG_MAX_HANDSHAKE_SIZE);

  dispatch_data_t data = dispatch_data_create(handshake, r.size,
                                              dispatch_get_main_queue(),
                                              DISPATCH_DATA_DESTRUCTOR_FREE);
  [self handleWireguard:(int)r.op withData:data];
}

- (void) handleOutbound:(NSData*)packet
           withProtocol:(int)protocol {
  if (!m_wireguard) {
    return;
  }

  // I think there is a small bug in boringtun to do with message padding.
  // The wireguard protocol states that the encapsulated packet must first
  // be padded out to a multiple of 16 bytes in length, but boringtun does
  // no such padding during encryption. So let's do it manually ourself.
  NSMutableData* plaintext = [NSMutableData dataWithData:packet];
  int tail = plaintext.length % 16;
  if (tail) {
    [plaintext increaseLengthBy:16 - tail];
  }

  // Encrypt the packet.
  size_t length = plaintext.length + WG_PACKET_OVERHEAD;
  uint8_t* ciphertext = (uint8_t*)malloc(length);
  struct wireguard_result r;
  r = wireguard_write(m_wireguard, (const uint8_t*)plaintext.bytes,
                      plaintext.length, ciphertext, length);

  dispatch_data_t data = dispatch_data_create(ciphertext, r.size,
                                              dispatch_get_main_queue(),
                                              DISPATCH_DATA_DESTRUCTOR_FREE);
  [self handleWireguard:(int)r.op withData:data];
}

- (void) handleInbound {
  nw_connection_receive_message(self.connection,
                                ^(dispatch_data_t data, nw_content_context_t ctx, bool done, nw_error_t err){
    if (err) {
      CFErrorRef cfError = nw_error_copy_cf_error(err);
      NSLog(@"recv error: %@", (__bridge NSError *)cfError);
      CFRelease(cfError);
      return;
    }
    if (!data) {
      // No data? That was kinda weird - oh well, try again.
      NSLog(@"recv empty");
      [self handleInbound];
      return;
    }

    size_t length;
    const void *ciphertext;
    dispatch_data_t __unused map = dispatch_data_create_map(data, &ciphertext, &length);
    uint8_t* plaintext = (uint8_t*)malloc(length);

    // Decrypt the wireguard packet.
    struct wireguard_result r;
    r = wireguard_read(m_wireguard, (const uint8_t*)ciphertext, length,
                       plaintext, length);

    // After processing a handshake response, update the lastHandshake time
    // if it looks and smells like the handshake was successful.
    if (*(const uint8_t*)ciphertext == 0x02) {
      struct stats wgStats = wireguard_stats(m_wireguard);
      if (wgStats.time_since_last_handshake < 0) {
        memset(&m_lastHandshake, 0, sizeof(m_lastHandshake));
      } else if (wgStats.time_since_last_handshake < 5) {
        clock_gettime(CLOCK_MONOTONIC, &m_lastHandshake);

        // Clear the handshake timeout
        memset(&m_handshakeTimeout, 0, sizeof(m_handshakeTimeout));

        // The conneciton is now up.
        if (m_completionHandler) {
          m_completionHandler(nil);
          m_completionHandler = nil;
        }
      }
    }

    dispatch_data_t packet = dispatch_data_create(plaintext, r.size,
                                                  dispatch_get_main_queue(),
                                                  DISPATCH_DATA_DESTRUCTOR_FREE);
    [self handleWireguard:(int)r.op withData:packet];

    // Keep going to receive more data.
    [self handleInbound];
  });
}

- (void) handleWireguard:(int)op
                withData:(dispatch_data_t)data {
  switch (op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      nw_connection_send(self.connection, data,
                         NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true,
                         ^(nw_error_t error) {
        if (error) {
          CFErrorRef cfError = nw_error_copy_cf_error(error);
          NSLog(@"wireguard send error: %@", (__bridge NSError*)cfError);
          CFRelease(cfError);
        }
      });
      break;

    case WIREGUARD_ERROR:
      NSLog(@"wireguard error");
      break;

    case WRITE_TO_TUNNEL_IPV4:
      [[fallthrough]];
    case WRITE_TO_TUNNEL_IPV6:
      NSData* packet = (NSData*)data;
      uint32_t header = (op == WRITE_TO_TUNNEL_IPV6) ? htonl(AF_INET6) : htonl(AF_INET);
      const struct iovec iov[2] = {
        {.iov_base = &header, .iov_len = sizeof(header)},
        {.iov_base = (void *)packet.bytes, .iov_len = packet.length},
      };
      int err = writev(m_tunfd, iov, 2);
      NSLog(@"utun write: %d", err);
      break;
  }
}

- (void)shutdownTunnel:(NSString*)desc withErrno:(int)code {
  [self shutdownTunnel:errorFromErrno(code, desc)];
}

- (void)shutdownTunnel:(NSError*)error {
  if (error) {
    NSLog(@"wireguard shutdown: %@", error);
  } else {
    NSLog(@"wireguard shutdown");
  }
  self.connection = nil;
  self.virtualInterface = nil;

  if (m_timer) {
    CFRunLoopRemoveTimer(CFRunLoopGetMain(), m_timer, kCFRunLoopDefaultMode);
    CFRelease(m_timer);
    m_timer = nil;
  }

  if (m_socket) {
    CFSocketInvalidate(m_socket);
    CFRelease(m_socket);
    m_socket = nil;
  }

  if (m_source) {
    CFRunLoopRemoveSource(CFRunLoopGetMain(), m_source, kCFRunLoopDefaultMode);
    CFRelease(m_source);
    m_source = nil;
  }

  if (m_tunfd >= 0) {
    close(m_tunfd);
    m_tunfd = -1;
  }

  if (m_wireguard) {
    tunnel_free(m_wireguard);
    m_wireguard = nil;
  }

  if (m_completionHandler) {
    m_completionHandler(error);
    m_completionHandler = nil;
  }
}

- (void)cancelTunnelWithError:(NSError*)error {
  [self shutdownTunnel:error];
}

- (WireguardStats*)getStatus {
  WireguardStats* result = [WireguardStats new];

  // Get the handshake time from the timestamp of the last received handshake
  // response packet, this will yeild better precision than the stats we get
  // from the boringtun API.
  if (m_lastHandshake.tv_sec || m_lastHandshake.tv_nsec) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    NSTimeInterval diff = (m_lastHandshake.tv_sec - now.tv_sec);
    diff += (m_lastHandshake.tv_nsec - now.tv_nsec) / (1000000000.0);
    result.lastHandshake = [NSDate dateWithTimeIntervalSinceNow:diff];
  }

  // Fetch the rest of the stats directly from boringtun.
  struct stats wgStats = wireguard_stats(m_wireguard);
  result.txBytes = wgStats.tx_bytes;
  result.rxBytes = wgStats.rx_bytes;
  result.estimatedLoss = wgStats.estimated_loss;
  result.estimatedRtt = wgStats.estimated_rtt;
  return result;
}

- (void)setMtu:(NSUInteger)mtu {
  _mtu = mtu;
  if (m_tunfd < 0) {
    return;
  }

  struct ifreq ifr;
  socklen_t ifnamesize = sizeof(ifr.ifr_name);
  int err = getsockopt(m_tunfd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifr.ifr_name,
                       &ifnamesize);
  if (err < 0) {
    NSLog(@"mtu failed to find ifname: %s", strerror(errno));
    return;
  }

  ifr.ifr_mtu = _mtu;
  if (ioctl(m_tunfd, SIOCSIFMTU, &ifr) != 0) {
    NSLog(@"mtu update failed: %s", strerror(errno));
  }
}

@end
