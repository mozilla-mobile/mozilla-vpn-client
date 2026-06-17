/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#import "interfaceconfig.h"
#import "utils.h"
#import "wireguardtunnel.h"

#include <atomic>
#include <arpa/inet.h>
#include <libkern/OSAtomic.h>

#include <net/if.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

@interface VPNSplitTunnelProvider : NETransparentProxyProvider

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *))completionHandler;

- (void)stopProxyWithReason:(NEProviderStopReason)reason
          completionHandler:(void (^)(void))completionHandler;

- (BOOL)handleNewFlow:(NEAppProxyFlow *)flow;

- (BOOL)matchExcludedFlow:(NEAppProxyFlow *)flow;

@property (strong) NETransparentProxyNetworkSettings* settings;
@property (strong) WireguardTunnel* wireguard;
@property (strong) InterfaceConfig* config;

@property (strong) NSSet* vpnDisabledApps;

@property (strong) NSTask* dnsManager;

@end

@implementation VPNSplitTunnelProvider {
  std::atomic_uint64_t m_handledTcpFlows;
  std::atomic_uint64_t m_handledUdpFlows;
  std::atomic_uint64_t m_handledUnknown;
}

- (id)init{
  self = [super init];
  NSLog(@"init proxy class");

  self.vpnDisabledApps = [NSSet set];

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  return self;
}

+ (nw_endpoint_t)convertEndpoint:(NWEndpoint*)old {
  if (old == nil) {
    return nil;
  } else if ([old isKindOfClass:[NWBonjourServiceEndpoint class]]) {
    NWBonjourServiceEndpoint* service = (NWBonjourServiceEndpoint*)old;
    return nw_endpoint_create_bonjour_service([service.name UTF8String],
                                              [service.type UTF8String],
                                              [service.domain UTF8String]);
  } else if (![old isKindOfClass:[NWHostEndpoint class]]) {
    // Some endpoint type we don't support.
    return nil;
  }
  NWHostEndpoint* host = (NWHostEndpoint*)old;
  
  // If possible, try to convert it into an address endpoint.
  int port = host.port.intValue;
  if ([host.hostname containsString:@":"]) {
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(sin6);
    sin6.sin6_port = htons(port);
    if (inet_pton(AF_INET6, host.hostname.UTF8String, &sin6.sin6_addr.s6_addr)) {
      return nw_endpoint_create_address((struct sockaddr*)&sin6);
    }
  } else {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(sin);
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host.hostname.UTF8String);
    if (sin.sin_addr.s_addr != INADDR_NONE) {
      return nw_endpoint_create_address((struct sockaddr*)&sin);
    }
  }

  return nw_endpoint_create_host(host.hostname.UTF8String, host.port.UTF8String);
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

- (void)startDnsManager:(NEDNSSettings*)settings {
  // Encode the settings as the arguments to the dnsmanager.
  NSMutableArray* args = [NSMutableArray arrayWithObject:@"dnsmanager"];
  [args addObjectsFromArray:settings.servers];

  // Setup the new DNS manager task.
  NSTask* task = [NSTask new];
  NSPipe* pipe = [NSPipe new];
  task.standardError = pipe;
  task.standardOutput = pipe;
  task.executableURL = [[NSBundle mainBundle] executableURL];
  task.arguments = args;

  // Forward the output from the pipe to the logs.
  pipe.fileHandleForReading.readabilityHandler = ^(NSFileHandle* handle){
    NSData* data = handle.availableData;
    // Check for EOF.
    if (data.length == 0) {
      handle.readabilityHandler = nil;
      return;
    }

    // Parse the buffer as a sequence of newline-terminated UTF-8 strings.
    NSString* buffer = [[NSString alloc] initWithData:data
                                             encoding:NSUTF8StringEncoding];
    NSCharacterSet* newlines = [NSCharacterSet newlineCharacterSet];
    NSCharacterSet* whitespaces = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    for (NSString* line in [buffer componentsSeparatedByCharactersInSet:newlines]) {
      NSString* trimmed = [line stringByTrimmingCharactersInSet:whitespaces];
      if (trimmed.length) {
        NSLog(@"dnsmanager: %@", trimmed);
      }
    }
  };

  if (self.dnsManager) {
    NSTask* prev = self.dnsManager;
    self.dnsManager = task;

    // Terminate the previous task and then launch the DNS manager.
    prev.terminationHandler = ^(NSTask* task){
      NSError* err;
      if (![task launchAndReturnError:&err]) {
        NSLog(@"dns manager failed to launch: %@", err);
      }
    };
    [prev terminate];
  } else {
    // Launch the DNS manager to reconfigure the system DNS.
    NSError* err;
    self.dnsManager = task;
    if (![task launchAndReturnError:&err]) {
      NSLog(@"dns manager failed to launch: %@", err);
    }
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

  // Parse the configuration
  _config = [[InterfaceConfig alloc] initFromDict:options];
  if (!self.config) {
    completionHandler(vpnProviderError(NEProviderStopReasonConfigurationFailed));
    return;
  }

  self.wireguard = [WireguardTunnel new];
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
  NSArray* apps = [options objectForKey:@"apps"];
  if ([apps isKindOfClass:[NSArray class]]) {
    NSPredicate* p = [NSPredicate predicateWithFormat:@"self isKindOfClass: %@", [NSString class]];
    self.vpnDisabledApps = [NSSet setWithArray:[apps filteredArrayUsingPredicate:p]];
  } else {
    self.vpnDisabledApps = [NSSet set];
  }

  // Configure the settings.
  __unsafe_unretained VPNSplitTunnelProvider* weakSelf = self;
  [self setTunnelNetworkSettings: self.settings
               completionHandler:^(NSError* error){
    if (error != nil) {
      NSLog(@"settings error: %@", error.localizedDescription);
      completionHandler(error);
    } else {
      NSLog(@"settings applied");
      [weakSelf.wireguard startTunnelWithOptions:weakSelf.config
                               completionHandler:^(NSError* wgError){
        if (wgError != nil) {
          completionHandler(wgError);
          return;
        }

        // Start the DNS manager, if configured.
        if (weakSelf.config.dnsSettings) {
          [weakSelf startDnsManager:weakSelf.config.dnsSettings];
        }

        // Register a KVO observer to switch servers upon configuration change.
        [weakSelf addObserver:weakSelf
                   forKeyPath:@"protocolConfiguration"
                      options:NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
                      context:nil];

        // Success
        completionHandler(nil);
      }];
    }
  }];
}

- (void)stopProxyWithReason:(NEProviderStopReason)reason 
          completionHandler:(void (^)(void))completionHandler {
  NSLog(@"stopping proxy");

  NSLog(@"handled tcp flows: %lld", std::atomic_load(&m_handledTcpFlows));
  NSLog(@"handled udp flows: %lld", std::atomic_load(&m_handledUdpFlows));
  NSLog(@"handled unknown flows: %lld", std::atomic_load(&m_handledUnknown));

  [self removeObserver:self
            forKeyPath:@"protocolConfiguration"];

  if (self.dnsManager) {
    [self.dnsManager terminate];
    self.dnsManager = nil;
  }

  [self.wireguard stopTunnelWithReason:reason
                     completionHandler:completionHandler];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
  // The only thing we should be observing is the protocolConfiguration
  if (![keyPath isEqual:@"protocolConfiguration"]) {
    return;
  }
  NSLog(@"configuration changed");

  // Parse and update the configuration
  NETunnelProviderProtocol* proto = (NETunnelProviderProtocol*)self.protocolConfiguration;
  _config = [[InterfaceConfig alloc] initFromDict:proto.providerConfiguration];
  if (!self.config) {
    // We can't make sense of this configuration.
    [self.wireguard stopTunnelWithReason:NEProviderStopReasonConfigurationFailed
                       completionHandler:^(){
      [self cancelProxyWithError:vpnProviderError(NEProviderStopReasonConfigurationFailed)];
    }];
    return;
  }

  // If the applications being excluded have changed, then we need to restart
  // the transparent proxy so that connections from the applications get closed
  // and re-evaluated.
  NSArray<NSString*>* apps = [proto.providerConfiguration objectForKey:@"apps"];
  NSPredicate* p = [NSPredicate predicateWithFormat:@"self isKindOfClass: %@", [NSString class]];
  NSSet* newDisabledApps;
  if ([apps isKindOfClass:[NSArray class]]) {
    newDisabledApps = [NSSet setWithArray:[apps filteredArrayUsingPredicate:p]];
  } else {
    newDisabledApps = [NSSet set];
  }

  // TODO: It would be nice if there was a way we could automatically restart
  // ourself in this case, but I can't figure out how to do this.
  if (![self.vpnDisabledApps isEqualToSet:newDisabledApps]) {
    [self stopProxyWithReason:NEProviderStopReasonSuperceded completionHandler:^(){
      [self cancelProxyWithError:vpnProviderError(NEProviderStopReasonSuperceded)];
    }];
    return;
  }

  // (Re)start the DNS manager, if configured.
  if (self.config.dnsSettings) {
    [self startDnsManager:self.config.dnsSettings];
  } else if (self.dnsManager) {
    // Otherwise, stop the DNS manager if it was running.
    [self.dnsManager terminate];
    self.dnsManager = nil;
  }

  // Check if the server identitiy changed. Do nothing if no changes.
  NETunnelProviderProtocol* old = [change objectForKey:@"old"];
  if (old && [old isKindOfClass:NETunnelProviderProtocol.class]) {
    NSString* oldPubKey = [old.providerConfiguration objectForKey:@"serverPublicKey"];
    if (oldPubKey && [oldPubKey isKindOfClass:NSString.class]) {
      if ([oldPubKey isEqual:self.config.serverPublicKey]) {
        return;
      }
    }
  }

  // Shutdown the old wireguard peer and start a new one.
  self.reasserting = TRUE;
  [self.wireguard.peer stopWithReason:NEProviderStopReasonSuperceded
                    completionHandler:^(){
    // Create and start a new peer.
    self.wireguard.peer = [[WireguardPeer alloc] initWithOptions:self.config
                                                       andTunnel:self.wireguard];
    [self.wireguard.peer startWithOptions:self.config
                        completionHandler:^(NSError*err){
      if (err) {
        [self cancelProxyWithError:err];
      } else {
        self.reasserting = FALSE;
      }
    }];
  }];
}

- (BOOL)matchExcludedFlow:(NEAppProxyFlow*)flow {
  // Without metadata - always direct the flow into the VPN.
  if (flow.metaData == nil) {
    return YES;
  }

  // If not signed - always direct the flow into the VPN.
  if (flow.metaData.sourceAppSigningIdentifier == nil) {
#ifdef MZ_DEBUG
    NSLog(@"new flow: unsigned -> %@", flow.remoteHostname);
#endif
    return YES;
  }

  NSString* sourceId = flow.metaData.sourceAppSigningIdentifier;
#ifdef MZ_DEBUG
  NSLog(@"new flow: %@ -> %@", sourceId, flow.remoteHostname);
#endif

  for (NSString* appId in self.vpnDisabledApps) {
    if (sourceId.length < appId.length) {
      continue;
    }
    // The source application can also be a child of the application id.
    // for example: "com.example.foo.bar" would match "com.example.foo"
    if ((sourceId.length > appId.length) &&
        ([sourceId characterAtIndex:appId.length] != '.')) {
      continue;
    }

    NSRange range = NSMakeRange(0, sourceId.length);
    NSComparisonResult result = [sourceId compare:appId options:NSLiteralSearch range:range];
    if (result == NSOrderedSame) {
      return NO;
    }
  }

  // No application matches this signing identifier - direct the flow into the VPN.
  return YES;
}

- (BOOL)handleNewFlow:(NEAppProxyFlow*) flow {
  if (![self matchExcludedFlow:flow]) {
    return NO;
  }

  // Redirect these flows into the VPN.
  flow.networkInterface = self.wireguard.virtualInterface;
  if ([flow isKindOfClass:[NEAppProxyTCPFlow class]]) {
    std::atomic_fetch_add(&m_handledTcpFlows, 1);
  } else if ([flow isKindOfClass:[NEAppProxyUDPFlow class]]) {
    std::atomic_fetch_add(&m_handledUdpFlows, 1);
  } else {
    std::atomic_fetch_add(&m_handledUnknown, 1);
  }
  return NO;
}

- (void)handleAppMessage:(NSData *)messageData
       completionHandler:(void (^)(NSData*)) completionHandler {
  NSError* error;
  NSKeyedUnarchiver* msg =
    [[NSKeyedUnarchiver alloc] initForReadingFromData:messageData
                                                error:&error];
  if (error != nil) {
      NSLog(@"app message error: %@", error.localizedDescription);
      [VPNSplitTunnelProvider sendAppResponse:error completionHandler:completionHandler];
      return;
  }
  NSString* action = [msg decodeObjectOfClass:NSString.class forKey:@"action"];
  if (!action) {
      NSLog(@"app message invalid action");
      NSError* error = vpnProviderError(NEProviderStopReasonConfigurationFailed);
      [VPNSplitTunnelProvider sendAppResponse:error completionHandler:completionHandler];
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

  // Wireguard Tunnel messages
  if ([action isEqualToString:@"status"]) {
    [VPNSplitTunnelProvider sendAppResponse:self.wireguard.status
                          completionHandler:completionHandler];
    return;
  }

  [VPNSplitTunnelProvider sendAppResponse:nil completionHandler:completionHandler];
}

+ (void)sendAppResponse:(id) obj
      completionHandler:(void (^)(NSData*)) completionHandler {
  if (!completionHandler) {
    return;
  }

  NSKeyedArchiver* encoder = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
  if ([obj isKindOfClass:[NSError class]]) {
    [encoder encodeObject:obj forKey:@"error"];
  } else if ([obj respondsToSelector:@selector(encodeWithCoder:)]) {
    [obj encodeWithCoder: encoder];
  }
  [encoder finishEncoding];
  completionHandler(encoder.encodedData);
}

@end
