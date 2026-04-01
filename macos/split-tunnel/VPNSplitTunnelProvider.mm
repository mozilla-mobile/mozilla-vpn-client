/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#include <atomic>
#include <arpa/inet.h>
#include <libkern/OSAtomic.h>

@interface VPNSplitTunnelProvider : NETransparentProxyProvider

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> * _Nullable)options
            completionHandler:(void (^ _Nonnull)(NSError * _Nullable))completionHandler;

- (void)stopProxyWithReason:(NEProviderStopReason)reason
          completionHandler:(void (^ _Nonnull)(void))completionHandler;

- (BOOL)handleNewFlow:(NEAppProxyFlow * _Nonnull)flow;

@property (retain) NETransparentProxyNetworkSettings* settings;

@end

@implementation VPNSplitTunnelProvider {
    std::atomic_uint64_t m_handledTcpFlows;
    std::atomic_uint64_t m_handledUdpFlows;
    std::atomic_uint64_t m_handledUnknown;

    dispatch_queue_t _queue;
}

- (id)init{
  self = [super init];
  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;
  NSLog(@"init proxy class");
  return self;
}

+ (NENetworkRule*) matchRoute:(NSString*)dest
                    andPrefix:(NSUInteger)prefix {
  NENetworkRule* rule = [NENetworkRule alloc];
  if (@available(macOS 15.0, *)) {
    // NENetworkRule has a different API starting with macOS 15.
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));

    if ([dest containsString:@":"]) {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
      sin6->sin6_family = AF_INET6;
      sin6->sin6_len = sizeof(struct sockaddr_in6);
      sin6->sin6_port = 0;
      inet_pton(AF_INET, dest.UTF8String, &sin6->sin6_addr.s6_addr);
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

  self.settings = [[NETransparentProxyNetworkSettings alloc] initWithTunnelRemoteAddress:self.protocolConfiguration.serverAddress];

  // Configure the proxy to capture all traffic
  if (@available(macOS 15.0, *)) {
    NENetworkRule* includeAllRule =
        [[NENetworkRule alloc] initWithRemoteNetworkEndpoint:nil
                                                remotePrefix:0
                                        localNetworkEndpoint:nil
                                                 localPrefix:0
                                                    protocol:NENetworkRuleProtocolAny
                                                   direction:NETrafficDirectionOutbound];
    self.settings.includedNetworkRules = @[includeAllRule];
  } else {
    self.settings = [[NETransparentProxyNetworkSettings alloc] initWithTunnelRemoteAddress:self.protocolConfiguration.serverAddress];
    NENetworkRule* includeAllRule =
        [[NENetworkRule alloc] initWithRemoteNetwork:nil
                                        remotePrefix:0
                                        localNetwork:nil
                                        localPrefix:0
                                           protocol:NENetworkRuleProtocolAny
                                          direction:NETrafficDirectionOutbound];
    self.settings.includedNetworkRules = @[includeAllRule];
  }

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

  _queue = dispatch_queue_create(nil, nil);

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
    NSLog(@"handled tcp flows: %lld", std::atomic_load(&m_handledTcpFlows));
    NSLog(@"handled udp flows: %lld", std::atomic_load(&m_handledUdpFlows));
    NSLog(@"handled unknown flows: %lld", std::atomic_load(&m_handledUnknown));
    completionHandler();
}

- (BOOL)handleNewFlow: (NEAppProxyFlow*) flow {
#if 1
  NSLog(@"handle flow");
  if (flow.metaData != nil) {
    NSLog(@"metadata sourceAppUniqueIdentifier: %@", flow.metaData.sourceAppUniqueIdentifier);
    NSLog(@"metadata sourceAppSigningIdentifier: %@", flow.metaData.sourceAppSigningIdentifier);
    NSLog(@"metadata filterFlowIdentifier: %@", flow.metaData.filterFlowIdentifier);
  }
#endif
  if ([flow isKindOfClass:[NEAppProxyTCPFlow class]]) {
    std::atomic_fetch_add(&m_handledTcpFlows, 1);
  } else if ([flow isKindOfClass:[NEAppProxyUDPFlow class]]) {
    std::atomic_fetch_add(&m_handledUdpFlows, 1);
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
       completionHandler:(void (^)(NSData* responseData)) completionHandler {
    NSError* error;
    NSKeyedUnarchiver* msg = [NSKeyedUnarchiver alloc];
    [msg initForReadingFromData:messageData
                          error:&error];
    if (error != nil) {
        NSLog(@"app message error: %@", error.localizedDescription);
        //completionHandler(nil);
        return;
    }
    NSObject* objAction = [msg decodeObjectForKey:@"action"];
    if (![objAction isKindOfClass:[NSString class]]) {
        NSLog(@"app message invalid action");
        //completionHandler(nil);
        return;
    }
    [msg finishDecoding];

    NSString* action = static_cast<NSString*>(objAction);
    NSLog(@"app message: %@", action);
    //completionHandler(nil);
}

@end
