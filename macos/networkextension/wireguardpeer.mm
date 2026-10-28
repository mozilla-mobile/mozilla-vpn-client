/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "wireguardpeer.h"

#include <sys/random.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "transport/lwo.h"
#include "transport/tcp.h"
#include "transport/udp.h"
#include "transport/wgtransport.h"
#include "utils.h"
#include "wireguardtunnel.h"

extern "C" {
#include "wireguard_ffi.h"
};

constexpr const int64_t PEER_WORKQUEUE_TIMEOUT = 30;
constexpr const size_t PEER_DATAGRAM_BUFSIZE = 4096;

@implementation WireguardPeer {
  // The wireguard tunnel provided by boringtun.
  struct wireguard_tunnel*  m_wireguard;

  CFRunLoopTimerRef         m_timer;
  struct timespec           m_lastHandshake;
  struct timespec           m_handshakeTimeout;

  int                       m_tunfd;
  NSThread*                 m_worker;

  // Routes owned by this peer.
  NSArray<RoutePrefix*>*    m_routes;

  // The completion handler to run on initial handshake or timeout. 
  void (^m_completionHandler)(NSError *error);
}

static void wgTimerCallback(CFRunLoopTimerRef t, void *info) {
  WireguardPeer* peer = (__bridge WireguardPeer*)info;
  [peer handleTimer];
}

- (id) initWithOptions:(WireguardConfig*) options
             andTunnel:(WireguardTunnel*) tunnel {
  self = [super init];
  self->m_tunfd = tunnel.tunfd;
  self->_tunnel = tunnel;

  // Prepare the wireguard transport
  switch (options.obfuscation) {
    case kObfuscationNone:
      self->_transport = [[UdpTransport alloc] initWithEndpoint:options.serverIpv4Addr];
      break;

    case kObfuscationLWO:
    case kObfuscationLWOv2:
      self->_transport = [[LwoTransport alloc] initWithOptions:options];
      break;

    case kObfuscationTCP:
      self->_transport = [[TcpTransport alloc] initWithEndpoint:options.serverIpv4Addr];
      break;

    default:
      // Unsupported transport.
      return nil;
  }

  // Create the wireguard tunnel.
  uint32_t index;
  getentropy(&index, sizeof(index));
  m_wireguard = new_tunnel(options.privateKey.UTF8String,
                           options.serverPublicKey.UTF8String,
                           nil, // Preshared key
                           300, // Keepalive period
                           index % (1U << 24));
  if (!m_wireguard) {
    return nil;
  }

  // Give the socket its own worker.
  m_worker = [[NSThread alloc] initWithTarget:self
                                     selector:@selector(socketWorker:)
                                       object:nil];
  return self;
}

- (void) dealloc {
  NSLog(@"wireguard peer dealloc");

  if (m_wireguard) {
    tunnel_free(m_wireguard);
  }

#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

- (void) startWithOptions:(WireguardConfig*) options
        completionHandler:(void (^)(NSError *error)) completionHandler {
#ifdef MZ_DEBUG
  char *addrstr = nw_endpoint_copy_address_string(options.serverIpv4Addr);
  NSLog(@"wireguard peer: %s port=%d", addrstr, nw_endpoint_get_port(options.serverIpv4Addr));
  free(addrstr);
#endif

  if ([self.transport connect] < 0) {
    completionHandler(vpnPosixError(errno, @"socket connect failed"));
    return;
  }

  // Configure routes into the tunnel interface.
  m_routes = [NSArray arrayWithArray:options.routes];
  for (RoutePrefix* prefix in m_routes) {
    [self.tunnel addRoute:prefix];
  }

  // Start the timer.
  CFRunLoopTimerContext timerContext = { .info = (__bridge void *)self };
  m_timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 0, 0.1, 0, 0,
                                 wgTimerCallback, &timerContext);
  CFRunLoopAddTimer(CFRunLoopGetMain(), m_timer, kCFRunLoopDefaultMode);

  // Force an initial handshake.
  [self renegotiate:completionHandler];

  // Start the socket worker
  [m_worker start];
}

- (void) socketWorker:(id)arg {
  NSLog(@"socket worker started");

  NSThread* thread = [NSThread currentThread];
  while (!thread.cancelled) {
    NSMutableData* dgram = [NSMutableData dataWithLength:PEER_DATAGRAM_BUFSIZE];
    int rx = [self.transport recv:dgram];
    if (rx == 0) {
      // Socket has closed.
      NSLog(@"socket closed");
      return;
    }
    if (rx < 0) {
      // Socket error occurred.
      if (errno == EINTR) continue;
      if (errno == EAGAIN) continue;
      NSLog(@"socket error: %s", strerror(errno));
      return;
    }
    if (rx < WG_MSG_LEN_MIN) {
      // Packet truncated.
      continue;
    }

    const uint8_t* packet = (const uint8_t *)dgram.bytes;
    uint8_t plaintext[PEER_DATAGRAM_BUFSIZE];
    struct wireguard_result result;
    result = wireguard_read(m_wireguard, packet, dgram.length, plaintext,
                            PEER_DATAGRAM_BUFSIZE);
    [self handleWireguard:result withBuffer:plaintext];

    // After processing a handshake response, update the lastHandshake time
    // if it looks and smells like the handshake was successful.
    if (packet[0] == WG_MSG_TYPE_HANDSHAKE_RESP) {
      struct stats wgStats = wireguard_stats(m_wireguard);
      if (wgStats.time_since_last_handshake < 0) {
        memset(&m_lastHandshake, 0, sizeof(m_lastHandshake));
      } else if (wgStats.time_since_last_handshake < 5) {
        clock_gettime(CLOCK_MONOTONIC, &m_lastHandshake);
        memset(&m_handshakeTimeout, 0, sizeof(m_handshakeTimeout));

        // The conneciton is now up. Run the completion handler (from the main thread).
        dispatch_async(dispatch_get_main_queue(), ^(){
          if (m_completionHandler) {
            m_completionHandler(nil);
            m_completionHandler = nil;
          }
        });
      }
    }
  }
}

- (void) stopWithReason:(NEProviderStopReason)reason 
      completionHandler:(void (^)()) completionHandler {
  [self cancelWithError:vpnProviderError(reason)];
  completionHandler();
}

- (void) cancelWithError:(NSError*)error {
  [self.transport shutdown];
  [m_worker cancel];

  // Drop routes into the tunnel.
  for (RoutePrefix* prefix in m_routes) {
    [self.tunnel removeRoute:prefix];
  }

  if (m_timer) {
    CFRunLoopRemoveTimer(CFRunLoopGetMain(), m_timer, kCFRunLoopDefaultMode);
    CFRelease(m_timer);
    m_timer = nil;
  }

  if (m_completionHandler) {
    m_completionHandler(error);
    m_completionHandler = nil;
  }
}

- (void) renegotiate:(void (^)(NSError *error)) completionHandler {
  NSLog(@"wireguard renegotiate");
  uint8_t handshake[WG_MAX_HANDSHAKE_SIZE];

  struct wireguard_result result;
  result = wireguard_force_handshake(m_wireguard, handshake, WG_MAX_HANDSHAKE_SIZE);

  // Set a timeout for the handshake to finish.
  clock_gettime(CLOCK_MONOTONIC, &m_handshakeTimeout);
  m_handshakeTimeout.tv_sec += WG_MAX_HANDSHAKE_TIMEOUT;
  m_completionHandler = completionHandler;

  [self handleWireguard:result withBuffer:handshake];
}

- (void) writePacket:(int)protocol
            withData:(NSData*)data {
  uint8_t ciphertext[data.length + WG_PACKET_OVERHEAD];
  const uint8_t* plaintext = (const uint8_t*)data.bytes;
  struct wireguard_result result;
  result = wireguard_write(m_wireguard, plaintext, data.length,
                           ciphertext, sizeof(ciphertext));
  [self handleWireguard:result withBuffer:ciphertext];
}

- (void) handleTimer {
  // Check for a handshake timeout.
  if (m_handshakeTimeout.tv_sec && m_handshakeTimeout.tv_nsec) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (now.tv_sec > m_handshakeTimeout.tv_sec) {
      [self cancelWithError:vpnPosixError(ETIMEDOUT, @"handshake timeout")];
      return;
    } else if (now.tv_sec < m_handshakeTimeout.tv_sec) {
      // It has not timed out.
    } else if (now.tv_nsec > m_handshakeTimeout.tv_nsec) {
      [self cancelWithError:vpnPosixError(ETIMEDOUT, @"handshake timeout")];
      return;
    }
  }

  uint8_t handshake[WG_MAX_HANDSHAKE_SIZE];
  struct wireguard_result result;
  result = wireguard_tick(m_wireguard, handshake, WG_MAX_HANDSHAKE_SIZE);
  [self handleWireguard:result withBuffer:handshake];
}

- (void) handleWireguard:(struct wireguard_result)result
              withBuffer:(const uint8_t*)data {
  uint32_t header = htonl(AF_INET);
  ssize_t err;
  switch (result.op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      [self.transport send:[NSData dataWithBytes:data length:result.size]];
      break;

    case WIREGUARD_ERROR:
      NSLog(@"peer error: %zu", result.size);
      break;

    case WRITE_TO_TUNNEL_IPV6:
      header = htonl(AF_INET6);
      [[fallthrough]];
    case WRITE_TO_TUNNEL_IPV4:
      struct iovec iov[2] = {
        {.iov_base = &header, .iov_len = sizeof(header)},
        {.iov_base = (void *)data, .iov_len = result.size},
      };
      const struct msghdr msg = {
        .msg_iov = iov,
        .msg_iovlen = 2,
      };
      ssize_t err = sendmsg(m_tunfd, &msg, MSG_DONTWAIT);
      break;
  }
}

- (WireguardStatus*)getStatus {
  WireguardStatus* result = [WireguardStatus new];

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

@end
