/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "wireguardpeer.h"

#include <sys/random.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"
#include "wireguardtunnel.h"

extern "C" {
#include "wireguard_ffi.h"
};

constexpr const int64_t PEER_WORKQUEUE_TIMEOUT = 30;
constexpr const size_t PEER_DATAGRAM_BUFSIZE = 4096;

constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_INIT = 0x01;
constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_RESP = 0x02;
constexpr const uint8_t WG_MSG_TYPE_COOKIE_REPLY = 0x03;
constexpr const uint8_t WG_MSG_TYPE_DATA = 0x04;

constexpr const size_t WG_MSG_LEN_MIN = 4;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_INIT = 148;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_RESP = 92;
constexpr const size_t WG_MSG_LEN_COOKIE_REPLY = 64;
constexpr const size_t WG_MSG_LEN_DATA_HEADER = 16;

@implementation WireguardPeer {
  // The wireguard tunnel provided by boringtun.
  struct wireguard_tunnel*  m_wireguard;

  CFRunLoopTimerRef         m_timer;
  struct timespec           m_lastHandshake;
  struct timespec           m_handshakeTimeout;

  int                       m_socket;
  int                       m_tunfd;
  NSThread*                 m_worker;

  // Routes owned by this peer.
  NSArray<RoutePrefix*>*    m_routes;

  // Server public key for LWO
  ObfuscationMethod         m_obfuscation;
  struct x25519_key         m_serverkey;
  struct x25519_key         m_clientkey;

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

  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_socket < 0) {
    return nil;
  }

  // Prepare obfuscation data.
  NSUInteger b64opts = NSDataBase64DecodingIgnoreUnknownCharacters;
  NSData* pubkey = [[NSData alloc] initWithBase64EncodedString:options.serverPublicKey options:b64opts];
  NSData* privkey = [[NSData alloc] initWithBase64EncodedString:options.privateKey options:b64opts];
  if (privkey.length >= 32) {
    // Determine our wireguard public key.
    struct x25519_key xprivate;
    [privkey getBytes:xprivate.key length:32];
    m_clientkey = x25519_public_key(xprivate);
  } else {
    return nil;
  }
  if (privkey.length >= 32) {
    [pubkey getBytes:m_serverkey.key length:32];
  } else {
    return nil;
  }
  m_obfuscation = kObfuscationLwoV2;

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
  if (m_socket >= 0) {
    close(m_socket);
  }
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

  const struct sockaddr* dest = nw_endpoint_get_address(options.serverIpv4Addr);
  if (connect(m_socket, dest, dest->sa_len) < 0) {
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
    int rx = read(m_socket, dgram.mutableBytes, dgram.length);
    if (rx == 0) {
      // Socket has closed.
      NSLog(@"socket closed");
      return;
    }
    if (rx < 0) {
      // Socket error occurred.
      if (errno == EINTR) continue;
      NSLog(@"socket error: %s", strerror(errno));
      return;
    }
    if (rx < WG_MSG_LEN_MIN) {
      // Packet truncated.
      continue;
    }
    dgram.length = rx;

    // Peek the LWO type and decapsulate if needed.
    uint8_t lwo;
    [dgram getBytes:&lwo range:NSMakeRange(1, 1)];
    if ((lwo & 0xc0) != 0) {
      if (![self decryptLwo:dgram]) {
        continue;
      }
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

- (BOOL) decryptLwo:(NSMutableData*)dgram {
  uint8_t* packet = (uint8_t *)dgram.mutableBytes;
  uint8_t lwo_type = packet[1] & 0xc0;
  if (lwo_type == 0x40) {
    // LWO Version 2.
    uint8_t len_mix = dgram.length & 0xff;
    uint8_t msg_type = packet[0] ^ (m_serverkey.key[0] + len_mix);

    // Validate bytes 1-3
    if (packet[1] != (((m_serverkey.key[0] + len_mix) & 0x3f) | 0x40)) return FALSE;
    if (packet[2] != ((m_serverkey.key[2] + len_mix + 2) & 0xff)) return FALSE;
    if (packet[3] != ((m_serverkey.key[3] + len_mix + 3) & 0xff)) return FALSE;
  
    // The protected area varies by packet type.
    uint8_t prot_len = 0;
    switch (msg_type) {
      case WG_MSG_TYPE_HANDSHAKE_INIT:
        prot_len = WG_MSG_LEN_HANDSHAKE_INIT;
        break;

      case WG_MSG_TYPE_HANDSHAKE_RESP:
        prot_len = WG_MSG_LEN_HANDSHAKE_RESP;
        break;

      case WG_MSG_TYPE_DATA:
        prot_len = WG_MSG_LEN_DATA_HEADER;
        break;

      default:
        // LWO decapsulation is not supported for this msg type.
        return FALSE;
    }

    if (dgram.length < prot_len) {
      // Packet appears to be truncated.
      return FALSE;
    }

    // Decrypt the protected area.
    for (int i = 0; i < prot_len; i++) {
      packet[i] ^= (m_serverkey.key[i % 32] + len_mix + (uint8_t)i);
    }
    packet[1] = 0x00;

    // Trim padding from non-data packets.
    if ((msg_type != WG_MSG_TYPE_DATA) && (dgram.length > prot_len)) {
      dgram.length = prot_len;
    }
    return TRUE;
  }

  if (lwo_type & 0x80) {
    // LWO Version 1.
    uint8_t msg_type = packet[0] ^ m_clientkey.key[0];
    uint8_t prot_len = 0;
    switch (msg_type) {
      case WG_MSG_TYPE_HANDSHAKE_INIT:
        prot_len = WG_MSG_LEN_HANDSHAKE_INIT;
        break;

      case WG_MSG_TYPE_HANDSHAKE_RESP:
        prot_len = WG_MSG_LEN_HANDSHAKE_RESP;
        break;

      case WG_MSG_TYPE_COOKIE_REPLY:
        prot_len = WG_MSG_LEN_COOKIE_REPLY;
        break;

      case WG_MSG_TYPE_DATA:
        // Is this a bug? The data header is only actually 16 bytes long, but
        // the Mullvad implementation obfuscates the first 32 bytes. 
        prot_len = 32;
        break;

      default:
        // LWO decapsulation is not supported for this msg type.
        return FALSE;
    }

    // Decrypt the protected area.
    for (int i = 0; i < prot_len; i++) {
      packet[i] ^= m_clientkey.key[i % 32];
    }
    packet[1] = 0;
    return TRUE;
  }

  // Otherwise, we don't understand this LWO type.
  return TRUE;
}

- (NSData*) encryptLwo:(NSData*)dgram
               version:(NSUInteger)version {
  const uint8_t* packet = (uint8_t *)dgram.bytes;
  uint8_t msg_type = packet[0];

  if (version >= 2) {
    // LWO Version 2
    uint8_t prot_len = 0;
    switch (msg_type) {
      case WG_MSG_TYPE_HANDSHAKE_INIT:
        prot_len = WG_MSG_LEN_HANDSHAKE_INIT;
        break;

      case WG_MSG_TYPE_HANDSHAKE_RESP:
        prot_len = WG_MSG_LEN_HANDSHAKE_RESP;
        break;

      case WG_MSG_TYPE_DATA:
        prot_len = WG_MSG_LEN_DATA_HEADER;
        break;

      default:
        // No encapsulation for these packets.
        return dgram;
    }

    // Prepare a copy of the packet for encoding.
    uint8_t padding[256];
    NSMutableData* ciphertext = [NSMutableData dataWithCapacity:dgram.length + sizeof(padding)];
    [ciphertext appendData:dgram];

    // Apply padding to non-data packets.
    if (msg_type != WG_MSG_TYPE_DATA) {
      size_t pad_len = 1 + (rand() % sizeof(padding));
      getentropy(padding, pad_len);
      [ciphertext appendBytes:padding length:pad_len];
    }

    // Obfuscate the protected region.
    uint8_t len_mix = ciphertext.length & 0xff;
    uint8_t* ctbytes = (uint8_t*)ciphertext.mutableBytes;
    for (int i = 0; i < prot_len; i++) {
      ctbytes[i] ^= (m_serverkey.key[i % 32] + len_mix + (uint8_t)i);
    }

    // Finalize byte 1.
    ctbytes[1] = ((m_serverkey.key[0] + len_mix) & 0x3f) | 0x40;
    return ciphertext;
  }

  if (version == 1) {
    // LWO Version 1
    uint8_t prot_len = 0;
    switch (msg_type) {
      case WG_MSG_TYPE_HANDSHAKE_INIT:
        prot_len = WG_MSG_LEN_HANDSHAKE_INIT;
        break;

      case WG_MSG_TYPE_HANDSHAKE_RESP:
        prot_len = WG_MSG_LEN_HANDSHAKE_RESP;
        break;

      case WG_MSG_TYPE_COOKIE_REPLY:
        prot_len = WG_MSG_LEN_COOKIE_REPLY;
        break;

      case WG_MSG_TYPE_DATA:
        // Is this a bug? The data header is only actually 16 bytes long, but
        // the Mullvad implementation obfuscates the first 32 bytes. 
        prot_len = 32;
        break;

      default:
        // LWO decapsulation is not supported for this msg type.
        return dgram;
    }

    // Obfuscate the protected region.
    NSMutableData* ciphertext = [NSMutableData dataWithData:dgram];
    uint8_t* ctbytes = (uint8_t*)ciphertext.mutableBytes;
    for (int i = 0; i < prot_len; i++) {
      ctbytes[i] ^= m_serverkey.key[i % 32];
    }

    // Finalize byte 1.
    getentropy(&ctbytes[1], 1);
    ctbytes[1] |= 0x80;
    return ciphertext;
  }

  // Otherwise - no LWO to perform.
  return dgram;
}

- (void) stopWithReason:(NEProviderStopReason)reason 
      completionHandler:(void (^)()) completionHandler {
  [self cancelWithError:vpnProviderError(reason)];
  completionHandler();
}

- (void) cancelWithError:(NSError*)error {
  if (m_worker) {
    [m_worker cancel];
  }
  shutdown(m_socket, SHUT_RDWR);

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
      [self sendToNetwork:[NSData dataWithBytes:data length:result.size]];
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

- (void) sendToNetwork:(NSData*)dgram {
  switch (m_obfuscation) {
    case kObfuscationNone:
      send(m_socket, dgram.bytes, dgram.length, MSG_DONTWAIT);
      break;

    case kObfuscationLwo:{
      NSData* lwo = [self encryptLwo:dgram version:1];
      send(m_socket, lwo.bytes, lwo.length, MSG_DONTWAIT);
      break;
    }

    case kObfuscationLwoV2:{
      NSData* lwo = [self encryptLwo:dgram version:2];
      send(m_socket, lwo.bytes, lwo.length, MSG_DONTWAIT);
      break;
    }

    default:
      // TODO: Obfuscation algorithms.
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
