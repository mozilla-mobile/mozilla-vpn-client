/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "lwo.h"

#include <sys/random.h>
#include <sys/socket.h>
#include <sys/uio.h>

#import "wgconfig.h"

extern "C" {
#include "wireguard_ffi.h"
};

@implementation LwoTransport {
  NSUInteger              m_version;
  struct x25519_key       m_serverkey;
  struct x25519_key       m_clientkey;
}

- (id) initWithOptions:(WgConfig*) options {
  self = [super initWithEndpoint:options.serverIpv4Addr];
  if (!self) {
    return nil;
  }

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

  return self;
}

- (NSMutableData*)recv {
  // Receive a packet from the UDP transport.
  NSMutableData *dgram = [super recv];
  if (!dgram) {
    return nil;
  }
  if (dgram.length < WG_MSG_LEN_MIN) {
    // Packet appears to be truncated.
    errno = EAGAIN;
    return nil;
  }

  // Check for LWO encryption, and decapsulate if required.
  uint8_t* packet = (uint8_t *)dgram.mutableBytes;
  uint8_t lwo_type = packet[1] & 0xc0;
  if (lwo_type == 0x40) {
    // LWO Version 2.
    uint8_t len_mix = dgram.length & 0xff;
    uint8_t msg_type = packet[0] ^ (m_serverkey.key[0] + len_mix);
    uint8_t marker = ((m_serverkey.key[0] + len_mix) & 0x3f) | 0x40;
    if (packet[1] != marker) {
      errno = EAGAIN;
      return nil;
    }

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
        errno = EAGAIN;
        return nil;
    }

    if (dgram.length < prot_len) {
      // Packet appears to be truncated.
      errno = EAGAIN;
      return nil;
    }

    // Decrypt the protected area.
    for (int i = 0; i < prot_len; i++) {
      packet[i] ^= (m_serverkey.key[i % 32] + len_mix + (uint8_t)i);
    }
    packet[1] = 0x00;

    // Verify that bytes 2-3 decoded as zero.
    if (packet[2] || packet[3]) {
      errno = EAGAIN;
      return nil;
    }

    // Trim padding from non-data packets.
    if ((msg_type != WG_MSG_TYPE_DATA) && (dgram.length > prot_len)) {
      dgram.length = prot_len;
    }
  } else if (lwo_type & 0x80) {
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
        errno = EAGAIN;
        return nil;
    }

    if (dgram.length < prot_len) {
      // Packet appears to be truncated.
      errno = EAGAIN;
      return nil;
    }

    // Decrypt the protected area.
    for (int i = 0; i < prot_len; i++) {
      packet[i] ^= m_clientkey.key[i % 32];
    }
    packet[1] = 0;
  }

  return dgram;
}

- (void)send:(NSData*)dgram {
  const uint8_t* packet = (const uint8_t *)dgram.bytes;
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
        [super send:dgram];
        return;
    }

    // Encrypt the packet header.
    uint8_t padding[256];
    uint8_t header[WG_MSG_LEN_HANDSHAKE_INIT];
    struct iovec iov[2];
    iov[0].iov_base = header;
    iov[0].iov_len = prot_len;

    if (msg_type != WG_MSG_TYPE_DATA) {
      iov[1].iov_base = padding;
      iov[1].iov_len = 1 + (rand() % sizeof(padding));
      getentropy(padding, iov[1].iov_len);
    } else {
      iov[1].iov_base = (uint8_t*)packet + WG_MSG_LEN_DATA_HEADER;
      iov[1].iov_len = dgram.length - WG_MSG_LEN_DATA_HEADER;
    }

    // Obfuscate the protected region.
    uint8_t len_mix = (iov[0].iov_len + iov[1].iov_len) & 0xff;
    uint8_t marker = ((m_serverkey.key[0] + len_mix) & 0x3f) | 0x40;
    for (int i = 0; i < prot_len; i++) {
      header[i] = packet[i] ^ (m_serverkey.key[i % 32] + len_mix + (uint8_t)i);
    }
    header[1] = marker;

    // Send the packet.
    struct msghdr msg = {};
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    sendmsg(self.sockfd, &msg, MSG_DONTWAIT);
  } else if (m_version == 1) {
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
        // No encapsulation for these packets.
        [super send:dgram];
        return;
    }

    // Obfuscate the protected region.
    uint8_t header[WG_MSG_LEN_HANDSHAKE_INIT];
    for (int i = 0; i < prot_len; i++) {
      header[i] = packet[i] ^ m_serverkey.key[i % 32];
    }
    // Finalize byte 1.
    header[1] = (rand() & 0x7f) | 0x80;

    struct iovec iov[2];
    iov[0].iov_base = header;
    iov[0].iov_len = prot_len;
    iov[1].iov_base = (uint8_t*)packet + prot_len;
    iov[1].iov_len = dgram.length - prot_len;

    struct msghdr msg = {};
    msg.msg_iov = iov;
    msg.msg_iovlen = (iov[1].iov_len) ? 2 : 1;
    sendmsg(self.sockfd, &msg, MSG_DONTWAIT);
  } else {
    // Otherwise - no LWO to perform.
    [super send:dgram];
  }
}

@end
