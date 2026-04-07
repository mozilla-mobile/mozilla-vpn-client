/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#import "bypasstcpflow.h"

#include <atomic>
#include <arpa/inet.h>
#include <libkern/OSAtomic.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

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

    // Detection of the VPN interface
    nw_path_monitor_t   m_pathMonitor;
    struct sockaddr_in  m_vpnIpv4Addr;
    struct sockaddr_in6 m_vpnIpv6Addr;
    nw_interface_t      m_vpnInterface;
}

- (id)init{
  self = [super init];
  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;
  NSLog(@"init proxy class");
  return self;
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

- (void)enumeratePath:(nw_path_t)path {
    NSLog(@"enumerating path");
    NSLog(@"ipv4: %d, ipv6: %d, dns: %d", nw_path_has_ipv4(path),
          nw_path_has_ipv6(path), nw_path_has_dns(path));

    nw_path_enumerate_interfaces(path, ^(nw_interface_t iface){
      // Fetch the interface's IPv4 address
      int sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
      if (sd < 0) {
        return true;
      }
      struct ifreq ifr;
      strncpy(ifr.ifr_name, nw_interface_get_name(iface), IFNAMSIZ);
      if (ioctl(sd, SIOCGIFADDR, &ifr) != 0) {
        close(sd);
        return true;
      }
      close(sd);

      if ((ifr.ifr_addr.sa_family != AF_INET) ||
          (ifr.ifr_addr.sa_len < sizeof(struct sockaddr_in))) {
        return true;
      }
      struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_addr;

      // Debug
      char buf[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
      NSLog(@"interface found: %s %s", ifr.ifr_name, buf);

      if (sin->sin_addr.s_addr == self->m_vpnIpv4Addr.sin_addr.s_addr) {
        // We found the VPN interface!
        m_vpnInterface = iface;

        // Debug
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
        NSLog(@"vpn found: %s %s", ifr.ifr_name, buf);
      }

      return true;
    });
}

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *error))completionHandler {
  NSLog(@"starting proxy");
  NSLog(@"config serverAddress: %@", self.protocolConfiguration.serverAddress);

  m_handledTcpFlows = 0;
  m_handledUdpFlows = 0;
  m_handledUnknown = 0;

  NSString* deviceIpv4Addr = [options objectForKey:@"deviceIpv4Address"];
  memset(&m_vpnIpv4Addr, 0, sizeof(m_vpnIpv4Addr));
  if (!deviceIpv4Addr) {
    completionHandler([VPNSplitTunnelProvider makeError:1 withDescription: @"No device IPv4 address"]);
    return;
  } else {
    NSString* addr = [deviceIpv4Addr componentsSeparatedByString:@"/"][0];
    m_vpnIpv4Addr.sin_family = AF_INET;
    m_vpnIpv4Addr.sin_len = sizeof(struct sockaddr_in);
    m_vpnIpv4Addr.sin_port = 0;
    inet_pton(AF_INET, addr.UTF8String, &m_vpnIpv4Addr.sin_addr.s_addr);
  }

  NSString* deviceIpv6Addr = [options objectForKey:@"deviceIpv6Address"];
  memset(&m_vpnIpv6Addr, 0, sizeof(m_vpnIpv6Addr));
  if (!deviceIpv6Addr) {
    completionHandler([VPNSplitTunnelProvider makeError:1 withDescription: @"No device IPv6 address"]);
    return;
  } else {
    NSString* addr = [deviceIpv6Addr componentsSeparatedByString:@"/"][0];
    m_vpnIpv6Addr.sin6_family = AF_INET6;
    m_vpnIpv6Addr.sin6_len = sizeof(struct sockaddr_in6);
    m_vpnIpv6Addr.sin6_port = 0;
    inet_pton(AF_INET6, addr.UTF8String, &m_vpnIpv6Addr.sin6_addr.s6_addr);
  }

  m_pathMonitor = nw_path_monitor_create();
  nw_path_monitor_set_update_handler(m_pathMonitor, ^(nw_path_t path){
    [self enumeratePath: path];
  });
  nw_path_monitor_start(m_pathMonitor);

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
    nw_path_monitor_cancel(m_pathMonitor);

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
    NEAppProxyTCPFlow* tcpFlow = (NEAppProxyTCPFlow*)flow;
    BypassTcpFlow* handler = [BypassTcpFlow createBypass:tcpFlow withInterface:m_vpnInterface];
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
