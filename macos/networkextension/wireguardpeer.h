/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>

#import "transport/wgtransport.h"
#import "wgconfig.h"
#import "wgstatus.h"

constexpr const int WG_PACKET_OVERHEAD = 32;
constexpr const int WG_PACKET_ALIGN = 16;
constexpr const int WG_MAX_HANDSHAKE_SIZE = 148;
constexpr const int WG_MAX_HANDSHAKE_TIMEOUT = 15;

extern "C" struct wireguard_tunnel;

@class WireguardTunnel;

@interface WireguardPeer : NSObject

- (id)initWithOptions:(WgConfig*)options andTunnel:(WireguardTunnel*)tunnel;

- (void)startWithOptions:(WgConfig*)options
       completionHandler:(void (^)(NSError* error))completionHandler;

- (void)stopWithReason:(NEProviderStopReason)reason completionHandler:(void (^)())completionHandler;

- (void)cancelWithError:(NSError*)error;

- (void)renegotiate:(void (^)(NSError* error))completionHandler;

- (void)writePacket:(int)protocol withData:(NSData*)data;

@property(strong, readonly, getter=getStatus) WgStatus* status;
@property(strong, readonly) NSObject<WgTransport>* transport;
@property(weak, readonly) WireguardTunnel* tunnel;
@end
