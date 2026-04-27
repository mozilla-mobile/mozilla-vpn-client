/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "interfaceconfig.h"

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

@end

@implementation InterfaceConfig
- (NSString*)findString:(NSString*)key {
  NSObject* value = [self.dict objectForKey:key];
  if (value == nil) {
    return nil;
  }
  if (![value isKindOfClass:[NSString class]]) {
    return nil;
  }
  return (NSString*)value;
}

- (nw_endpoint_t)findAddress:(NSString*)key
                    withPort:(NSUInteger)port {
  NSString* addr = [self findString:key];
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

- (id)parseFromDict:(NSDictionary<NSString *,id> *)dict {
  InterfaceConfig* config = [InterfaceConfig new];
  config->_dict = dict;

  config.privateKey = [config findString:@"privateKey"];
  if (!config.privateKey) {
    return nil;
  }
  config.deviceIpv4Addr = [config findAddress:@"deviceIpv4Addr" withPort:0];
  if ((!config.deviceIpv4Addr) ||
      (nw_endpoint_get_address(config.deviceIpv4Addr)->sa_family != AF_INET)) {
    return nil;
  }
  config.deviceIpv6Addr = [config findAddress:@"deviceIpv6Addr" withPort:0];
  if ((!config.deviceIpv6Addr) ||
      (nw_endpoint_get_address(config.deviceIpv6Addr)->sa_family != AF_INET6)) {
    return nil;
  }

  config.serverPublicKey = [config findString:@"serverPublicKey"];
  if (!config.serverPublicKey) {
    return nil;
  }

  NSObject* serverPort = [config.dict objectForKey:@"serverPort"];
  if ([serverPort isKindOfClass:[NSString class]]) {
    config.serverPort = [(NSString*)serverPort intValue];
  } else if ([serverPort isKindOfClass:[NSNumber class]]) {
    config.serverPort = [(NSNumber*)serverPort intValue];
  } else {
    // default wireguard port.
    config.serverPort = 51820;
  }

  config.serverIpv4Addr = [config findAddress:@"serverIpv4AddrIn" withPort:config.serverPort];
  config.serverIpv6Addr = [config findAddress:@"serverIpv6AddrIn" withPort:config.serverPort];

  // We don't actually use this, but parse it anyways.
  config.serverIpv4Gateway = [config findAddress:@"serverIpv4Gateway" withPort:0];
  config.serverIpv6Gateway = [config findAddress:@"serverIpv6Gateway" withPort:0];

  // Parse the allowed IP address ranges.
  NSObject* rangesObject = [config.dict objectForKey:@"routes"];
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
  config.routes = [NSArray arrayWithArray:routes];

  return config;
}

- (id)initFromCoder:(NSCoder*)coder {
  self = [super init];
  return [self parseFromDict:[[NSDictionary alloc] initWithCoder:coder]];
}

- (id)initFromDict:(NSDictionary<NSString *,id> *)dict {
  self = [super init];
  return [self parseFromDict:dict];
}

@end
