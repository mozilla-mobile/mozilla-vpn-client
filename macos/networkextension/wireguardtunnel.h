/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>

#import "interfaceconfig.h"
#import "wireguardpeer.h"
#import "wireguardstatus.h"

@interface WireguardTunnel : NSObject

- (void) startTunnelWithOptions:(InterfaceConfig*) options
              completionHandler:(void (^)(NSError *error)) completionHandler;

- (void) stopTunnelWithReason:(NEProviderStopReason)reason 
            completionHandler:(void (^)()) completionHandler;

- (void) cancelTunnelWithError:(NSError*)error;

- (NSError*) setTunnelAddress:(nw_endpoint_t)endpoint;

@property (nonatomic) NSUInteger mtu;
@property (strong) WireguardPeer* peer;
@property (strong, readonly, getter=getStatus) WireguardStatus* status;
@property (strong) nw_endpoint_t ipv4address;
@property (strong) nw_endpoint_t ipv6address;
@property (strong) nw_interface_t virtualInterface;
@end
