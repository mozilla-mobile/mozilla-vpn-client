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

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *))completionHandler;

- (void)stopProxyWithReason:(NEProviderStopReason)reason
          completionHandler:(void (^)(void))completionHandler;

- (BOOL)handleNewFlow:(NEAppProxyFlow *)flow;

- (BOOL)matchAppFlow:(NEAppProxyFlow *)flow;

- (void)defaultRouteChanged:(int)family
               viaInterface:(nw_interface_t)interface
                withGateway:(NSData*)gateway;

@property (strong) NETransparentProxyNetworkSettings* settings;
@property (strong) RouteManager* routeManager;
@property (strong) nw_interface_t ipv4Interface;
@property (strong) nw_interface_t ipv6Interface;
@property (strong) nw_interface_t vpnInterface;

@property (strong) NSMutableArray* vpnDisabledApps;

@end

@implementation VPNSplitTunnelProvider {
  std::atomic_uint64_t m_handledTcpFlows;
  std::atomic_uint64_t m_handledUdpFlows;
  std::atomic_uint64_t m_handledUnknown;
}

- (id)init{
  self = [super init];
  NSLog(@"init proxy class");

  self.vpnDisabledApps = [NSMutableArray new];

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  return self;
}

+ (NSError*) makeError:(NSInteger)code
       withDescription:(NSString*)desc {
  return [NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier]
                             code:code
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

    nw_endpoint_t dest = nw_endpoint_create_address((struct sockaddr*)&addr);
    return [rule initWithDestinationNetworkEndpoint:dest
                                             prefix:prefix
                                           protocol:NENetworkRuleProtocolAny];
  } else {
    // Deprecated API for macOS < 15.0
    NWHostEndpoint* host = [NWHostEndpoint endpointWithHostname:dest port:@"0"];
    return [rule initWithDestinationNetwork:host
                                     prefix:prefix
                                   protocol:NENetworkRuleProtocolAny];
  }
}

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *error))completionHandler {
  NSLog(@"starting proxy");
#ifdef MZ_DEBUG
  NSLog(@"config serverAddress: %@", self.protocolConfiguration.serverAddress);
#endif

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  // Start the route manager
  _routeManager = [RouteManager new];
  [self.routeManager startWithDelegate:self];

  self.settings = [[NETransparentProxyNetworkSettings alloc] initWithTunnelRemoteAddress:self.protocolConfiguration.serverAddress];

  // Configure the proxy to capture all traffic
  NENetworkRule* includeAllRule = nil;
  if (@available(macOS 15, *)) {
    includeAllRule =
        [[NENetworkRule alloc] initWithRemoteNetworkEndpoint:nil
                                                remotePrefix:0
                                        localNetworkEndpoint:nil
                                                 localPrefix:0
                                                    protocol:NENetworkRuleProtocolAny
                                                   direction:NETrafficDirectionOutbound];
  } else {
    includeAllRule =
        [[NENetworkRule alloc] initWithRemoteNetwork:nil
                                        remotePrefix:0
                                        localNetwork:nil
                                         localPrefix:0
                                            protocol:NENetworkRuleProtocolAny
                                           direction:NETrafficDirectionOutbound];
  }
  self.settings.includedNetworkRules = @[includeAllRule];

  auto excludeRules = [[NSMutableArray<NENetworkRule*> new] init];

  // Exclude LAN traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"10.0.0.0" andPrefix:8]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"172.16.0.0" andPrefix:12]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"192.168.0.0" andPrefix:16]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"fc00::" andPrefix:7]];

  // Exclude multicast traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"224.0.0.0" andPrefix:4]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"ff00::" andPrefix:8]];

  // Exclude link-local traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"169.254.0.0" andPrefix:16]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"fe80::" andPrefix:10]];

  // Exclude loopback traffic
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"127.0.0.0" andPrefix:8]];
  [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:@"::1" andPrefix:128]];

  // Exclude connections to the VPN server
  NSString* serverIpv4Addr = [options objectForKey:@"serverIpv4AddrIn"];
  if (serverIpv4Addr) {
    [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:serverIpv4Addr andPrefix:32]];
  }
  NSString* serverIpv6Addr = [options objectForKey:@"serverIpv6AddrIn"];
  if (serverIpv6Addr) {
    [excludeRules addObject:[VPNSplitTunnelProvider matchRoute:serverIpv6Addr andPrefix:128]];
  }

  self.settings.excludedNetworkRules = excludeRules;

  // Initialize the excluded application list.
  [self.vpnDisabledApps removeAllObjects];
  NSArray* apps = [options objectForKey:@"apps"];
  if (apps) {
    NSEnumerator* iter = [apps objectEnumerator];
    while (id appId = [iter nextObject]) {
      if (![appId isKindOfClass:[NSString class]]) {
        continue;
      }
#ifdef MZ_DEBUG
      NSLog(@"excluding app %@ from VPN", appId);
#endif
      [self.vpnDisabledApps addObject: appId];
    }
  }

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
      NSLog(@"clearing cloned ipv4 route");

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
      NSLog(@"clearing cloned ipv6 route");

      struct sockaddr_in6 sin6;
      memset(&sin6, 0, sizeof(sin6));
      sin6.sin6_family = AF_INET6;
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

- (BOOL)matchAppFlow:(NEAppProxyFlow*)flow {
  // Without metadata - do not exclude the application.
  if (flow.metaData == nil) {
    return NO;
  }

  // If not signed - do not exclude the application.
  if (flow.metaData.sourceAppSigningIdentifier == nil) {
#ifdef MZ_DEBUG
    NSLog(@"new flow: unsigned -> %@", flow.remoteHostname);
#endif
    return NO;
  }

  NSString* sourceId = flow.metaData.sourceAppSigningIdentifier;
#ifdef MZ_DEBUG
  NSLog(@"new flow: %@ -> %@", sourceId, flow.remoteHostname);
#endif

  NSEnumerator* iter = [self.vpnDisabledApps objectEnumerator];
  while (NSString* appId = [iter nextObject]) {
    if (sourceId.length < appId.length) {
      continue;
    }
    // The source application can also be a child of the application id.
    // for example: "com.example.foo.bar" would match "com.example.foo"
    if ((sourceId.length > appId.length) &&
        ([sourceId characterAtIndex:appId.length] != '.')) {
      continue;
    }

    NSComparisonResult result = [sourceId compare:appId
                                          options:NSLiteralSearch
                                            range:NSMakeRange(0, appId.length)];
    if (result == NSOrderedSame) {
      return YES;
    }
  }

  // No application matches this signing identifier.
  return NO;
}

- (BOOL)handleNewFlow:(NEAppProxyFlow*) flow {
  // Evaluate whether the source of this flow should be excluded from the VPN.
  if (![self matchAppFlow:flow]) {
    return NO;
  }

  // Perform flow bypassing.
  if ([flow isKindOfClass:[NEAppProxyTCPFlow class]]) {
    NEAppProxyTCPFlow* tcpFlow = (NEAppProxyTCPFlow*)flow;
    BypassTcpFlow* handler = [BypassTcpFlow createBypass:tcpFlow withInterface:self.ipv4Interface];
    if (!handler) {
      return NO;
    }

    [handler startBypass:^(NSError* error){
      if (error) {
        NSLog(@"flow closed with error: %@", error);
      }
    }];

    std::atomic_fetch_add(&m_handledTcpFlows, 1);
    return YES;
  } else if ([flow isKindOfClass:[NEAppProxyUDPFlow class]]) {
    NEAppProxyUDPFlow* udpFlow = (NEAppProxyUDPFlow*)flow;
    BypassUdpFlow* handler = [BypassUdpFlow createBypass:udpFlow withInterface:self.ipv4Interface];
    if (!handler) {
      return NO;
    }

    [handler startBypass:^(NSError* error){
      if (error) {
        NSLog(@"flow closed with error: %@", error);
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
  NSLog(@"cancel proxy: %@", error.localizedDescription);
}

- (void)handleAppMessage:(NSData *) messageData
       completionHandler:(void (^)(NSData*)) completionHandler {
  NSError* error;
  NSKeyedUnarchiver* msg =
    [[NSKeyedUnarchiver alloc] initForReadingFromData:messageData
                                                error:&error];
  if (error != nil) {
      NSLog(@"app message error: %@", error.localizedDescription);
      [VPNSplitTunnelProvider sendAppError:error completionHandler:completionHandler];
      return;
  }
  NSString* action = [msg decodeObjectOfClass:NSString.class forKey:@"action"];
  if (!action) {
      NSLog(@"app message invalid action");
      NSError* error = [VPNSplitTunnelProvider makeError:1 withDescription:@"invalid app message invalid"];
      [VPNSplitTunnelProvider sendAppError:error completionHandler:completionHandler];
      return;
  }

  NSMutableArray* apps = [NSMutableArray new];
  NSArray* msgAppList = [msg decodeObjectOfClass:NSArray.class forKey:@"apps"];
  if (msgAppList) {
    NSEnumerator* iter = [msgAppList objectEnumerator];
    while (id appId = [iter nextObject]) {
      if (![appId isKindOfClass:[NSString class]]) {
      continue;
      }
#ifdef MZ_DEBUG
      NSLog(@"msg %@ %@ from excluded application set", action, appId);
#endif
      [apps addObject:appId];
    }
  }
  [msg finishDecoding];

  if ([action isEqualToString: @"clear"]) {
    [self.vpnDisabledApps removeAllObjects];
  } else if ([action isEqualToString: @"add"]) {
    [self.vpnDisabledApps addObjectsFromArray:apps];
  } else if ([action isEqualToString: @"delete"]) {
    [self.vpnDisabledApps removeObjectsInArray:apps];
  } else {
    NSLog(@"unsupported app message: %@", action);  
  }

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
               viaInterface:(nw_interface_t)interface
                withGateway:(NSData*)gateway {
  int action = RTM_ADD;
  int ifindex = interface ? nw_interface_get_index(interface) : 0;

  if (family == AF_INET) {
    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_len = sizeof(dst);
    NSData* dstAddr = [NSData dataWithBytes:&dst length:sizeof(dst)];

    if (interface) {
      NSLog(@"default ipv4 route via %s", nw_interface_get_name(interface));
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
    } else if (self.ipv4Interface) {
      NSLog(@"default ipv4 route lost");
      action = RTM_DELETE;
      ifindex = nw_interface_get_index(self.ipv4Interface);
    }

    // Update the cloned IPv4 default route.
    self.ipv4Interface = interface;
    if (ifindex != 0) {
      [self.routeManager rtmSendRoute:action
                        toDestination:dstAddr
                          withPrefix:0
                        viaInterface:ifindex
                          withGateway:gateway
                            andFlags:RTF_IFSCOPE];
    }
  } else if (family == AF_INET6) {
    struct sockaddr_in6 dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin6_family = AF_INET6;
    dst.sin6_len = sizeof(dst);
    NSData* dstAddr = [NSData dataWithBytes:&dst length:sizeof(dst)];

    if (interface) {
      NSLog(@"default ipv6 route via %s", nw_interface_get_name(interface));

      if (!self.ipv6Interface) {
        action = RTM_ADD;
      } else if (ifindex == nw_interface_get_index(self.ipv6Interface)) {
        action = RTM_CHANGE;
      } else {
        action = RTM_ADD;
        [self.routeManager rtmSendRoute:RTM_DELETE
                          toDestination:dstAddr
                            withPrefix:0
                          viaInterface:nw_interface_get_index(self.ipv6Interface)
                            withGateway:nil
                              andFlags:RTF_IFSCOPE];
      }
    } else if (self.ipv6Interface) {
      NSLog(@"default ipv6 route lost");
      action = RTM_DELETE;
      ifindex = nw_interface_get_index(self.ipv6Interface);
    }

    // Update the cloned IPv6 default route.
    self.ipv6Interface = interface;
    if (ifindex != 0) {
      [self.routeManager rtmSendRoute:action
                        toDestination:dstAddr
                           withPrefix:0
                         viaInterface:ifindex
                          withGateway:gateway
                             andFlags:RTF_IFSCOPE];
    }
  }
}

@end
