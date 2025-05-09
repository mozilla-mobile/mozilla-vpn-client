/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#include <libkern/OSAtomic.h>

@interface VPNSplitTunnelProvider : NETransparentProxyProvider {
    int64_t m_handledTcpFlows;
    int64_t m_handledUdpFlows;
    int64_t m_handledUnknown;
}

@property (retain) NETransparentProxyNetworkSettings* settings;

@end

@implementation VPNSplitTunnelProvider

- (id)init{
  self = [super init];
  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;
  NSLog(@"init proxy class");
  return self;
}

// TODO: NENEtworkRule seems to have a completely different API for macOS 15
// and later, so this probably needs some rewriting and version-specific glue.
+ (NENetworkRule*) matchRoute:(NSString*)dest
                    andPrefix:(NSUInteger)prefix {
  NENetworkRule* rule = [NENetworkRule alloc];
  NWHostEndpoint* endpoint = [NWHostEndpoint endpointWithHostname:dest
                                                             port:@"0"];
  [rule initWithDestinationNetwork:endpoint
                            prefix:prefix
                          protocol:NENetworkRuleProtocolAny];
  return rule;
}

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *error))completionHandler {
  NSLog(@"starting proxy");
  NSLog(@"config serverAddress: %@", self.protocolConfiguration.serverAddress);

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  // Configure the proxy to capture all traffic
  self.settings = [[NETransparentProxyNetworkSettings alloc] initWithTunnelRemoteAddress:self.protocolConfiguration.serverAddress];
  NENetworkRule* includeAllRule =
      [[NENetworkRule alloc] initWithRemoteNetwork:nil
                                      remotePrefix:0
                                      localNetwork:nil
                                       localPrefix:0
                                          protocol:NENetworkRuleProtocolAny
                                         direction:NETrafficDirectionOutbound];
  self.settings.includedNetworkRules = @[includeAllRule];

  // Exclude LAN traffic
  NENetworkRule* ipv4lanA = [VPNSplitTunnelProvider matchRoute:@"10.0.0.0" andPrefix:8];
  NENetworkRule* ipv4lanB = [VPNSplitTunnelProvider matchRoute:@"172.16.0.0" andPrefix:12];
  NENetworkRule* ipv4lanC = [VPNSplitTunnelProvider matchRoute:@"192.168.0.0" andPrefix:16];
  NENetworkRule* ipv4mcast = [VPNSplitTunnelProvider matchRoute:@"224.0.0.0" andPrefix:4];
  NENetworkRule* ipv4local = [VPNSplitTunnelProvider matchRoute:@"127.0.0.0" andPrefix:8];
  self.settings.excludedNetworkRules = @[ipv4lanA, ipv4lanB, ipv4lanC, ipv4mcast, ipv4local];

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
    NSLog(@"handled tcp flows: %lld", m_handledTcpFlows);
    NSLog(@"handled udp flows: %lld", m_handledUdpFlows);
    NSLog(@"handled unknown flows: %lld", m_handledUnknown);
    completionHandler();
}

- (BOOL)handleNewFlow: (NEAppProxyFlow*) flow {
#if 0
  NSLog(@"handle flow");
  if (flow.metaData != nil) {
    NSLog(@"metadata sourceAppUniqueIdentifier: %@", flow.metaData.sourceAppUniqueIdentifier);
    NSLog(@"metadata sourceAppSigningIdentifier: %@", flow.metaData.sourceAppSigningIdentifier);
    NSLog(@"metadata filterFlowIdentifier: %@", flow.metaData.filterFlowIdentifier);
  }
#endif
  if ([flow isKindOfClass:[NEAppProxyTCPFlow class]]) {
    OSAtomicIncrement64(&m_handledTcpFlows);
  } else if ([flow isKindOfClass:[NEAppProxyUDPFlow class]]) {
    OSAtomicIncrement64(&m_handledUdpFlows);
  } else {
    OSAtomicIncrement64(&m_handledUnknown);
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
