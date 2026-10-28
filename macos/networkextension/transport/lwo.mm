/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "lwo.h"

#include <sys/random.h>
#include <sys/socket.h>

#import "wireguardconfig.h"

extern "C" {
#include "wireguard_ffi.h"
};

@implementation LwoTransport {
  int                     m_sockfd;
  struct sockaddr_storage m_destaddr;

  NSUInteger              m_version;
  struct x25519_key       m_serverkey;
  struct x25519_key       m_clientkey;
}

- (id) initWithOptions:(WireguardConfig*) options {
  self = [super init];

  NSUInteger b64opts = NSDataBase64DecodingIgnoreUnknownCharacters;
  NSData* privkey = [[NSData alloc] initWithBase64EncodedString:options.privateKey options:b64opts];
  if (privkey.length >= 32) {
    // Determine our wireguard public key.
    struct x25519_key xprivate;
    [privkey getBytes:xprivate.key length:32];
    m_clientkey = x25519_public_key(xprivate);
  } else {
    return nil;
  }

  NSData* pubkey = [[NSData alloc] initWithBase64EncodedString:options.serverPublicKey options:b64opts];
  if (pubkey.length >= 32) {
    [pubkey getBytes:m_serverkey.key length:32];
  } else {
    return nil;
  }

  if (options.obfuscation == kObfuscationLWOv2) {
    m_version = 2;
  } else if (options.obfuscation == kObfuscationLWO) {
    m_version = 1;
  } else {
    m_version = 0;
  }

  const struct sockaddr* dest = nw_endpoint_get_address(options.serverIpv4Addr);
  if (dest->sa_len > sizeof(m_destaddr)) {
    return nil;
  }
  memcpy(&m_destaddr, dest, dest->sa_len);

  m_sockfd = socket(dest->sa_family, SOCK_DGRAM, IPPROTO_UDP);
  if (m_sockfd < 0) {
    return nil;
  }

  return self;
}

- (void) dealloc {
  NSLog(@"lwo dealloc");
  close(m_sockfd);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

+ (BOOL) isObfuscated:(NSData*)dgram {
  uint8_t* packet = (uint8_t *)dgram.bytes;
  uint8_t lwo_type = packet[1] & 0xc0;
  return (lwo_type != 0x00);
}

- (BOOL) decrypt:(NSMutableData*)dgram {
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

- (NSData*) encrypt:(NSData*)dgram {
  const uint8_t* packet = (uint8_t *)dgram.bytes;
  uint8_t msg_type = packet[0];

  if (m_version >= 2) {
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

  if (m_version == 1) {
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

- (int)connect {
  return connect(m_sockfd, (struct sockaddr*)&m_destaddr, m_destaddr.ss_len);
}

- (void)shutdown {
  shutdown(m_sockfd, SHUT_RDWR);
}

- (void)send:(NSData*)dgram {
  NSData* lwo = [self encrypt:dgram];
  send(m_sockfd, lwo.bytes, lwo.length, MSG_DONTWAIT);
}

- (int)recv:(NSMutableData*)dgram {
  int rx = read(m_sockfd, dgram.mutableBytes, dgram.length);
  if (rx <= 0) {
    return rx;
  }
  dgram.length = rx;

  if (rx < WG_MSG_LEN_MIN) {
    // Packet appears to be truncated.
    errno = EAGAIN;
    return -1;
  }

  if ([LwoTransport isObfuscated:dgram]) {
    if (![self decrypt:dgram]) {
      // Decryption failed
      errno = EAGAIN;
      return -1;
    }
  }

  return dgram.length;
}

@end
