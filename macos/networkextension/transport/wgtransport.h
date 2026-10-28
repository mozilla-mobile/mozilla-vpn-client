/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGTRANSPORT_H

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

// Connect to the transport. Same semantics as the socket connect() function.
- (int) connect;

// Send a datagram to the transport. This must not block, and should be
// something like an atomic system call (eg: write()). If the packet
// cannot be sent immediately, it should be dropped.
- (void) send:(NSData*)dgram;

// Receive a datagram from the transport and copy it into a pre-allocated
// buffer. This call is permitted to block until a datagram is received.
//
// Returns <0 if an error occurred and will set errno accordingly.
// Returns 0 if the transport has closed gracefully.
// Otherwise returns the number of bytes received.
//
// Non fatal errors may include:
//    EAGAIN - No packet was ready, or the packet was silently dropped.
//    EINTR - Packet reception was interrupted by a signal.
//
// All other errors are considered fatal, and should result in a closure of the
// wireguard session.
- (int) recv:(NSMutableData*)dgram;

// Terminate the transport, closing sockets and reparing for garbage collection.
// If there is a recv() call in progress, it must terminate with return code of
// zero after calling this method.
- (void) shutdown;

// Return the packet overhead used by this transport. It must include the size
// of the network and transport headers (eg: IP and UDP), plus any additional
// capacity lost due to the obfuscation.
- (NSUInteger) overhead;

@end

#endif  // WGTRANSPORT_H
