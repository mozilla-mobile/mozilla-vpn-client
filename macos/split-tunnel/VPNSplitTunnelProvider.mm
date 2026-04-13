/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#import "bypasstcpflow.h"
#import "bypassudpflow.h"
#import "routemanager.h"

#include <atomic>
#include <arpa/inet.h>
#include <libkern/OSAtomic.h>

#include <net/if.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

@interface VPNSplitTunnelProvider : NETransparentProxyProvider<RouteManagerDelegate>

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> * _Nullable)options
            completionHandler:(void (^ _Nonnull)(NSError * _Nullable))completionHandler;

- (void)stopProxyWithReason:(NEProviderStopReason)reason
          completionHandler:(void (^ _Nonnull)(void))completionHandler;

- (BOOL)handleNewFlow:(NEAppProxyFlow * _Nonnull)flow;

- (void)defaultRouteChanged:(int)family
               viaInterface:(nw_interface_t)interface
                withGateway:(NSData*)gateway;

@property (strong) NETransparentProxyNetworkSettings* settings;
@property (strong) RouteManager* routeManager;
@property (strong) nw_interface_t ipv4Interface;
@property (strong) nw_interface_t ipv6Interface;
@property (strong) nw_interface_t vpnInterface;

@end

@implementation VPNSplitTunnelProvider {
    std::atomic_uint64_t m_handledTcpFlows;
    std::atomic_uint64_t m_handledUdpFlows;
    std::atomic_uint64_t m_handledUnknown;
}

- (id)init{
  self = [super init];
  NSLog(@"init proxy class");

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  return self;
}

- (void)dealloc {
  NSLog(@"destroy proxy class");
  [super dealloc];
}

+ (NSError*) makeError:(NSInteger)code
       withDescription:(NSString*)desc {
  return [NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier]
                             code:1
                         userInfo:@{NSLocalizedDescriptionKey: desc}];
}

+ (NENetworkRule*) matchRoute:(NSString*)dest
                    andPrefix:(NSUInteger)prefix {
  NENetworkRule* rule = [NENetworkRule alloc];
  if (@available(macOS 15, *)) {
    // NENetworkRule has a different API starting with macOS 15.
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));

    if ([dest containsString:@":"]) {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
      sin6->sin6_family = AF_INET6;
      sin6->sin6_len = sizeof(struct sockaddr_in6);
      sin6->sin6_port = 0;
      inet_pton(AF_INET6, dest.UTF8String, &sin6->sin6_addr.s6_addr);
    } else {
      struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
      sin->sin_family = AF_INET;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_port = 0;
      inet_pton(AF_INET, dest.UTF8String, &sin->sin_addr.s_addr);
    }

    nw_endpoint_t endpoint = nw_endpoint_create_address((struct sockaddr*)&addr);
    [rule initWithDestinationNetworkEndpoint:endpoint
                                      prefix:prefix
                                    protocol:NENetworkRuleProtocolAny];
  } else {
    // Deprecated API for macOS < 15.0
    NWHostEndpoint* endpoint = [NWHostEndpoint endpointWithHostname:dest
                                                               port:@"0"];
    [rule initWithDestinationNetwork:endpoint
                              prefix:prefix
                            protocol:NENetworkRuleProtocolAny];
  }

  return rule;
}

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *error))completionHandler {
  NSLog(@"starting proxy");
  NSLog(@"config serverAddress: %@", self.protocolConfiguration.serverAddress);

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  // Start the route manager
  _routeManager = [RouteManager new];
  [self.routeManager startWithDelegate:self];

  self.settings = [[NETransparentProxyNetworkSettings alloc] initWithTunnelRemoteAddress:self.protocolConfiguration.serverAddress];

  // Configure the proxy to capture all traffic
  NENetworkRule* includeAllRule =
    [[NENetworkRule alloc] initWithRemoteNetwork:nil
                                    remotePrefix:0
                                    localNetwork:nil
                                    localPrefix:0
                                       protocol:NENetworkRuleProtocolAny
                                      direction:NETrafficDirectionOutbound];
  self.settings.includedNetworkRules = @[includeAllRule];

  auto excludeRules = [[NSMutableArray<NENetworkRule*> new] init];

  // Exclude LAN traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"10.0.0.0" andPrefix:8]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"172.16.0.0" andPrefix:12]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"192.168.0.0" andPrefix:16]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"224.0.0.0" andPrefix:4]];

  // Exclude loopback traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"127.0.0.0" andPrefix:8]];

  // Exclude connections to the VPN server
  NSString* serverIpv4Addr = [options objectForKey:@"serverIpv4AddrIn"];
  if (serverIpv4Addr) {
    [excludeRules addObject: [VPNSplitTunnelProvider matchRoute:serverIpv4Addr andPrefix:32]];
  }
  NSString* serverIpv6Addr = [options objectForKey:@"serverIpv6AddrIn"];
  if (serverIpv6Addr) {
    [excludeRules addObject: [VPNSplitTunnelProvider matchRoute:serverIpv6Addr andPrefix:128]];
  }

  self.settings.excludedNetworkRules = excludeRules;

  // Configure the settings.
  [self setTunnelNetworkSettings: self.settings
               completionHandler:^(NSError* error){
    if (error != nil) {
      NSLog(@"settings error: %@", error.localizedDescription);
    } else {
      NSLog(@"settings applied");
    }
    completionHandler(error);
  }];
}

- (void)stopProxyWithReason:(NEProviderStopReason)reason 
          completionHandler:(void (^)(void))completionHandler {
    NSLog(@"stopping proxy");

    // Remove captured default routes, if known.
    if (self.ipv4Interface) {
      struct sockaddr_in sin;
      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_len = sizeof(sin);
      NSData* dst = [NSData dataWithBytes:&sin length:sizeof(sin)];

      [self.routeManager rtmSendRoute:RTM_DELETE
                        toDestination:dst
                           withPrefix:0
                         viaInterface:nw_interface_get_index(self.ipv4Interface)
                          withGateway:nil
                             andFlags:RTF_IFSCOPE];

      self.ipv4Interface = nil;
    }
    if (self.ipv6Interface) {
      struct sockaddr_in6 sin6;
      memset(&sin6, 0, sizeof(sin6));
      sin6.sin6_family = AF_INET;
      sin6.sin6_len = sizeof(sin6);
      NSData* dst = [NSData dataWithBytes:&sin6 length:sizeof(sin6)];

      [self.routeManager rtmSendRoute:RTM_DELETE
                        toDestination:dst
                           withPrefix:0
                         viaInterface:nw_interface_get_index(self.ipv6Interface)
                          withGateway:nil
                             andFlags:RTF_IFSCOPE];

      self.ipv6Interface = nil;
    }
    self.routeManager = nil;

    NSLog(@"handled tcp flows: %lld", std::atomic_load(&m_handledTcpFlows));
    NSLog(@"handled udp flows: %lld", std::atomic_load(&m_handledUdpFlows));
    NSLog(@"handled unknown flows: %lld", std::atomic_load(&m_handledUnknown));

    completionHandler();
}

- (BOOL)handleNewFlow: (NEAppProxyFlow*) flow {
  if (flow.metaData == nil) {
    return NO;
  }

#if 1
  NSLog(@"metadata sourceAppUniqueIdentifier: %@", flow.metaData.sourceAppUniqueIdentifier);
  NSLog(@"metadata sourceAppSigningIdentifier: %@", flow.metaData.sourceAppSigningIdentifier);
  NSLog(@"metadata filterFlowIdentifier: %@", flow.metaData.filterFlowIdentifier);
  if (flow.remoteHostname) {
    NSLog(@"metadata remoteHostname: %@", flow.remoteHostname);
  }
#endif

  // For the purposes of testing. Only handle flows from com.apple.safari
  if ([flow.metaData.sourceAppSigningIdentifier compare:@"com.apple.Safari"] != NSOrderedSame) {
    return NO;
  }

  if ([flow isKindOfClass:[NEAppProxyTCPFlow class]]) {
    NEAppProxyTCPFlow* tcpFlow = (NEAppProxyTCPFlow*)flow;
    BypassTcpFlow* handler = [BypassTcpFlow createBypass:tcpFlow withInterface:self.ipv4Interface];
    if (!handler) {
      // No handling required for this flow.
      return NO;
    }

    [handler startBypass:tcpFlow completionHandler:^(NSError* error){
      if (error) {
        NSLog(@"flow closed with error: %@", error);
      } else {
        NSLog(@"flow closed succefully");
      }
    }];

    std::atomic_fetch_add(&m_handledTcpFlows, 1);
    return YES;
  } else if ([flow isKindOfClass:[NEAppProxyUDPFlow class]]) {
    NEAppProxyUDPFlow* udpFlow = (NEAppProxyUDPFlow*)flow;
    BypassUdpFlow* handler = [BypassUdpFlow createBypass:udpFlow withInterface:self.ipv4Interface];
    if (!handler) {
      // No handling required for this flow.
      return NO;
    }

    [handler startBypass:^(NSError* error){
      if (error) {
        NSLog(@"flow closed with error: %@", error);
      } else {
        NSLog(@"flow closed succefully");
      }
    }];

    std::atomic_fetch_add(&m_handledUdpFlows, 1);
    return YES;
  } else {
    std::atomic_fetch_add(&m_handledUnknown, 1);
  }
  return NO;
}

- (void)cancelProxyWithError:(NSError *) error {
    // TODO: Implement Me!
    NSLog(@"cancel proxy: %@", error.localizedDescription);
}

- (void)handleAppMessage:(NSData *) messageData
       completionHandler:(void (^)(NSData*)) completionHandler {
    NSError* error;
    NSKeyedUnarchiver* msg = [NSKeyedUnarchiver alloc];
    [msg initForReadingFromData:messageData
                          error:&error];
    if (error != nil) {
        NSLog(@"app message error: %@", error.localizedDescription);
        [VPNSplitTunnelProvider sendAppError:error completionHandler:completionHandler];
        return;
    }
    NSString* action = [msg decodeObjectOfClass:NSString.class
                                         forKey:@"action"];
    if (!action) {
        NSLog(@"app message invalid action");
        NSError* error = [VPNSplitTunnelProvider makeError:1 withDescription:@"invalid app message invalid"];
        [VPNSplitTunnelProvider sendAppError:error completionHandler:completionHandler];
        return;
    }
    [msg finishDecoding];

    NSLog(@"app message: %@", action);
    [VPNSplitTunnelProvider sendAppResponse:nil completionHandler:completionHandler];
}

+ (void)sendAppResponse:(NSData*) responseData
      completionHandler:(void (^)(NSData*)) completionHandler {
  if (!completionHandler) {
    return;
  }
  completionHandler(responseData);
}

+ (void)sendAppError:(NSError*) error
   completionHandler:(void (^)(NSData*)) completionHandler {
  if (!completionHandler) {
    return;
  }
  NSKeyedArchiver* encoder = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
  [encoder encodeObject:error forKey:@"error"];
  [encoder finishEncoding];
  completionHandler(encoder.encodedData);
}

- (void)defaultRouteChanged:(int)family
               viaInterface:(nw_interface_t)iface
                withGateway:(NSData*)gateway {
  if (family == AF_INET) {
    int action = RTM_ADD;
    int ifindex = 0;

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_len = sizeof(dst);
    NSData* dstAddr = [NSData dataWithBytes:&dst
                                     length:sizeof(dst)];

    if (iface) {
      NSLog(@"default ipv4 route via %s", nw_interface_get_name(iface));

      ifindex = nw_interface_get_index(iface);
      if (!self.ipv4Interface) {
        action = RTM_ADD;
      } else if (ifindex == nw_interface_get_index(self.ipv4Interface)) {
        action = RTM_CHANGE;
      } else {
        action = RTM_ADD;
        [self.routeManager rtmSendRoute:RTM_DELETE
                          toDestination:dstAddr
                            withPrefix:0
                          viaInterface:nw_interface_get_index(self.ipv4Interface)
                            withGateway:nil
                              andFlags:RTF_IFSCOPE];
      }

      self.ipv4Interface = iface;
    } else {
      NSLog(@"default ipv4 route lost");
      if (self.ipv4Interface) {
        action = RTM_DELETE;
        ifindex = nw_interface_get_index(self.ipv4Interface);
        self.ipv4Interface = nil;
      }
    }

    // Update the cloned IPv4 default route.
    if (ifindex != 0) {
      [self.routeManager rtmSendRoute:action
                        toDestination:dstAddr
                          withPrefix:0
                        viaInterface:ifindex
                          withGateway:gateway
                            andFlags:RTF_IFSCOPE];
    }
  } else if (family == AF_INET6) {
    //m_ipv6Interface = iface;
    if (iface) {
      NSLog(@"default ipv6 route via %s", nw_interface_get_name(iface));
    } else {
      NSLog(@"default ipv6 route lost");
    }
  }
}

@end
