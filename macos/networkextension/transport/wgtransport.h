/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>

#include <stdint.h>

constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_INIT = 0x01;
constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_RESP = 0x02;
constexpr const uint8_t WG_MSG_TYPE_COOKIE_REPLY = 0x03;
constexpr const uint8_t WG_MSG_TYPE_DATA = 0x04;

constexpr const size_t WG_MSG_LEN_MIN = 4;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_INIT = 148;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_RESP = 92;
constexpr const size_t WG_MSG_LEN_COOKIE_REPLY = 64;
constexpr const size_t WG_MSG_LEN_DATA_HEADER = 16;

@protocol WgTransport

// Connect to the transport. This call is permitted to block until such
// time as the transport connects and is ready to send and receive datagrams.
// 
// Returns -1 if an error occurred and will set errno accordingly, otherwise
// this returns zero on success.
- (int)connect;

// Send a datagram to the transport. This must not block, and should be
// something like an atomic system call (eg: write()). If the packet
// cannot be sent immediately, it should be dropped.
- (void)send:(NSData*)dgram;

// Receive a datagram from the transport. This call is permitted to block
// until a datagram is received.
//
// Returns nil if an error occurred and will set errno accordingly.
//
// Non fatal errors may include:
//    EAGAIN - No packet was ready, or the packet was silently dropped.
//    EINTR - Packet reception was interrupted by a signal.
//    ESHUTDOWN - socket has shutdown.
//
// All other errors are considered fatal, and should result in a closure of the
// wireguard session.
- (NSMutableData*)recv;

// Terminate the transport and prepare for garbage collection. If there is a
// recv() call in progress, it must terminate and return an ESHUTDOWN error
// code after calling this method.
- (void)shutdown;

// Return the packet overhead used by this transport. It must include the size
// of the network and transport headers (eg: IP and UDP), plus any additional
// capacity lost due to the obfuscation.
- (NSUInteger)overhead;

@end
