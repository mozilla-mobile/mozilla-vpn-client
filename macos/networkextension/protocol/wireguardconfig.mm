/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "wireguardconfig.h"

#import <Foundation/Foundation.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

@implementation RoutePrefix {
  struct sockaddr_storage m_storage;
}

+ (id)parseRoute:(NSString*)routeString {
  RoutePrefix* route = [RoutePrefix new];

  NSArray<NSString*>* split = [routeString componentsSeparatedByString:@"/"];
  NSString* dest = split[0];

  if ([dest containsString:@":"]) {
    // IPv6 address
    struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&route->m_storage;
    memset(sin6, 0, sizeof(struct sockaddr_in6));
    sin6->sin6_family = AF_INET6;
    sin6->sin6_len = sizeof(struct sockaddr_in6);
    sin6->sin6_port = 0; // Not used.
    if (!inet_pton(AF_INET6, dest.UTF8String, &sin6->sin6_addr.s6_addr)) {
      return nil;
    }
    route->_prefixLength = 128;
  } else {
    struct sockaddr_in* sin = (struct sockaddr_in*)&route->m_storage;
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_port = 0; // Not used.
    if (!inet_pton(AF_INET, dest.UTF8String, &sin->sin_addr.s_addr)) {
      return nil;
    }
    route->_prefixLength = 32;
  }

  if (split.count > 1) {
    NSInteger i = split[1].integerValue;
    if ((i < 0) || (i > route.prefixLength)) {
      return nil;
    }
    route->_prefixLength = i;
  }

  return route;
}

- (const struct sockaddr*)getDestination {
  return (const struct sockaddr*)&m_storage;
}

- (NSString*)getString {
  if (m_storage.ss_family == AF_INET) {
    char buf[INET_ADDRSTRLEN];
    const struct sockaddr_in* sin = (struct sockaddr_in*)&m_storage;
    if (inet_ntop(AF_INET, &sin->sin_addr.s_addr, buf, sizeof(buf))) {
      return [NSString stringWithFormat:@"%s/%lu", buf, (unsigned long)_prefixLength];
    }
  }

  if (m_storage.ss_family == AF_INET6) {
    char buf[INET6_ADDRSTRLEN];
    const struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&m_storage;
    if (inet_ntop(AF_INET6, &sin6->sin6_addr.s6_addr, buf, sizeof(buf))) {
      return [NSString stringWithFormat:@"%s/%lu", buf, (unsigned long)_prefixLength];
    }
  }

  return @"";
}

@end

@implementation WireguardConfig
+ (NSString*)findString:(NSDictionary<NSString *,id> *)dict
                 forKey:(NSString*)key {
  NSObject* value = [dict objectForKey:key];
  if (value == nil) {
    return nil;
  }
  if (![value isKindOfClass:[NSString class]]) {
    return nil;
  }
  return (NSString*)value;
}

+ (nw_endpoint_t)findAddress:(NSDictionary<NSString *,id> *)dict
                      forKey:(NSString*)key
                    withPort:(NSUInteger)port {
  NSString* addr = [WireguardConfig findString:dict forKey:key];
  if (!addr) {
    return nil;
  }
  // Strip the prefix length, if present.
  addr = [addr componentsSeparatedByString:@"/"][0];

  if ([addr containsString:@":"]) {
    // IPv6 address
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(struct sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(struct sockaddr_in6);
    sin6.sin6_port = htons(port);
    if (!inet_pton(AF_INET6, addr.UTF8String, &sin6.sin6_addr.s6_addr)) {
      return nil;
    }
    return nw_endpoint_create_address((struct sockaddr*)&sin6);
  } else {
    // IPv4 address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(struct sockaddr_in);
    sin.sin_port = htons(port);
    if (!inet_pton(AF_INET, addr.UTF8String, &sin.sin_addr.s_addr)) {
      return nil;
    }
    return nw_endpoint_create_address((struct sockaddr*)&sin);
  }
}

- (id)initFromDict:(NSDictionary<NSString *,id> *)dict {
  self = [super init];

  self.privateKey = [WireguardConfig findString:dict forKey:@"privateKey"];
  if (!self.privateKey) {
    return nil;
  }
  self.deviceIpv4Addr = [WireguardConfig findAddress:dict forKey:@"deviceIpv4Addr" withPort:0];
  if ((!self.deviceIpv4Addr) ||
      (nw_endpoint_get_address(self.deviceIpv4Addr)->sa_family != AF_INET)) {
    return nil;
  }
  self.deviceIpv6Addr = [WireguardConfig findAddress:dict forKey:@"deviceIpv6Addr" withPort:0];
  if ((!self.deviceIpv6Addr) ||
      (nw_endpoint_get_address(self.deviceIpv6Addr)->sa_family != AF_INET6)) {
    return nil;
  }

  self.serverPublicKey = [WireguardConfig findString:dict forKey:@"serverPublicKey"];
  if (!self.serverPublicKey) {
    return nil;
  }

  NSObject* serverPort = [dict objectForKey:@"serverPort"];
  if ([serverPort isKindOfClass:[NSString class]]) {
    self.serverPort = [(NSString*)serverPort intValue];
  } else if ([serverPort isKindOfClass:[NSNumber class]]) {
    self.serverPort = [(NSNumber*)serverPort intValue];
  } else {
    // default wireguard port.
    self.serverPort = 51820;
  }

  self.serverIpv4Addr = [WireguardConfig findAddress:dict forKey:@"serverIpv4AddrIn" withPort:self.serverPort];
  self.serverIpv6Addr = [WireguardConfig findAddress:dict forKey:@"serverIpv6AddrIn" withPort:self.serverPort];

  // We don't actually use this, but parse it anyways.
  self.serverIpv4Gateway = [WireguardConfig findAddress:dict forKey:@"serverIpv4Gateway" withPort:0];
  self.serverIpv6Gateway = [WireguardConfig findAddress:dict forKey:@"serverIpv6Gateway" withPort:0];

  // Parse the allowed IP address ranges.
  NSObject* rangesObject = [dict objectForKey:@"routes"];
  NSMutableArray<RoutePrefix*>* routes = [NSMutableArray new];
  if ([rangesObject isKindOfClass:[NSArray<NSString*> class]]) {
    NSArray<NSString*>* list = (NSArray<NSString*>*)rangesObject;
    for (NSString* rangeString in list) {
      RoutePrefix* prefix = [RoutePrefix parseRoute:rangeString];
      if (!prefix) {
        return nil;
      }
      [routes addObject:prefix];
    }
  }
  self.routes = [NSArray arrayWithArray:routes];

  // Parse the DNS servers list.
  NSObject* dnsObject = [dict objectForKey:@"dnsSettings"];
  self.dnsSettings = nil;
  if ([dnsObject isKindOfClass:[NEDNSSettings class]]) {
    self.dnsSettings = (NEDNSSettings*)dnsObject;
  } else if ([dnsObject isKindOfClass:[NSArray<NSString*> class]]) {
    // Parse and validate the addresses.
    NSMutableArray<NSString*>* dnsServers = [NSMutableArray new];
    for (NSString* server in (NSArray*)dnsObject) {
      if ([server containsString:@":"]) {
        struct in6_addr addr;
        if (inet_pton(AF_INET6, server.UTF8String, &addr)) {
          [dnsServers addObject:server];
        }
      } else {
        struct in_addr addr;
        if (inet_pton(AF_INET, server.UTF8String, &addr)) {
          [dnsServers addObject:server];
        }
      }
    }

    if (dnsServers.count > 0) {
      NEDNSSettings* settings = [NEDNSSettings alloc];
      self.dnsSettings = [settings initWithServers:dnsServers];
    }
  }

  NSArray* apps = [dict objectForKey:@"apps"];
  if ([apps isKindOfClass:[NSArray class]]) {
    NSPredicate* p = [NSPredicate predicateWithFormat:@"self isKindOfClass: %@", [NSString class]];
    self.excludedApps = [NSSet setWithArray:[apps filteredArrayUsingPredicate:p]];
  } else {
    self.excludedApps = [NSSet set];
  }

  return self;
}

+ (NSString*)encodeAddr:(nw_endpoint_t)ep {
  char* s = nw_endpoint_copy_address_string(ep);
  return [[NSString alloc] initWithBytesNoCopy:s
                                        length:strlen(s)
                                      encoding:NSASCIIStringEncoding
                                  freeWhenDone:true];
}

- (void)encodeWithDict:(NSMutableDictionary<NSString*,id> *)dict {
  [dict setObject:self.privateKey forKey:@"privateKey"];
  [dict setObject:[WireguardConfig encodeAddr:self.deviceIpv4Addr] forKey:@"deviceIpv4Addr"];
  [dict setObject:[WireguardConfig encodeAddr:self.deviceIpv4Addr]forKey:@"deviceIpv6Addr"];

  [dict setObject:self.serverPublicKey forKey:@"serverPublicKey"];
  [dict setObject:[WireguardConfig encodeAddr:self.serverIpv4Addr] forKey:@"serverIpv4AddrIn"];
  [dict setObject:[WireguardConfig encodeAddr:self.serverIpv6Addr] forKey:@"serverIpv6AddrIn"];
  [dict setObject:[WireguardConfig encodeAddr:self.serverIpv4Gateway] forKey:@"serverIpv4Gateway"];
  [dict setObject:[WireguardConfig encodeAddr:self.serverIpv6Gateway] forKey:@"serverIpv6Gateway"];
  [dict setObject:[NSNumber numberWithInt:self.serverPort] forKey:@"serverPort"];

  if (self.dnsSettings) {
    [dict setObject:self.dnsSettings.servers.firstObject forKey:@"dnsSettings"];
  }

  NSMutableArray* ipAddressRanges = [NSMutableArray arrayWithCapacity:self.routes.count];
  for (RoutePrefix* range in self.routes) {
    [ipAddressRanges addObject:range.string];
  }
  [dict setObject:ipAddressRanges forKey:@"routes"];

  [dict setObject:self.excludedApps.allObjects forKey:@"apps"];
}

- (NSDictionary<NSString*,id>*)encodeToDict {
  NSMutableDictionary* dict = [NSMutableDictionary dictionary];
  [self encodeWithDict:dict];
  return dict;
}

@end
