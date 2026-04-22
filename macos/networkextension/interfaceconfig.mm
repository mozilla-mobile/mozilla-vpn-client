/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "interfaceconfig.h"

#import <Foundation/Foundation.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

@implementation RoutePrefix
// Nothing to do here
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

+ (id)parseFromCoder:(NSCoder*)coder {
  return [InterfaceConfig parseFromDict:[[NSDictionary alloc] initWithCoder:coder]];
}

+ (id)parseFromDict:(NSDictionary<NSString *,id> *)dict {
  InterfaceConfig* config = [InterfaceConfig new];
  config->_dict = dict;

  config.privateKey = [config findString:@"privateKey"];
  if (!config.privateKey) {
    return nil;
  }
  NSString* deviceIpv4Addr = [config findString:@"deviceIpv4Addr"];
  if (!deviceIpv4Addr) {
    return nil;
  } else {
    struct sockaddr_in sin;
    NSString* addr = [deviceIpv4Addr componentsSeparatedByString:@"/"][0];

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(sin);
    if (!inet_pton(AF_INET, addr.UTF8String, &sin.sin_addr.s_addr)) {
      return nil;
    }
    config.deviceIpv4Addr = nw_endpoint_create_address((struct sockaddr*)&sin);
  }
  NSString* deviceIpv6Addr = [config findString:@"deviceIpv6Addr"];
  if (!deviceIpv6Addr) {
    return nil;
  } else {
    struct sockaddr_in6 sin6;
    NSString* addr = [deviceIpv6Addr componentsSeparatedByString:@"/"][0];

    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(sin6);
    if (!inet_pton(AF_INET6, addr.UTF8String, &sin6.sin6_addr.s6_addr)) {
      return nil;
    }
    config.deviceIpv6Addr = nw_endpoint_create_address((struct sockaddr*)&sin6);
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

  NSString* serverIpv4Addr = [config findString:@"serverIpv4AddrIn"];
  if (serverIpv4Addr) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_len = sizeof(sin);
    sin.sin_port = htons(config.serverPort);
    if (!inet_pton(AF_INET, serverIpv4Addr.UTF8String, &sin.sin_addr.s_addr)) {
      return nil;
    }

    config.serverIpv4Addr = nw_endpoint_create_address((struct sockaddr*)&sin);
  }

  NSString* serverIpv6Addr = [config findString:@"serverIpv6AddrIn"];
  if (serverIpv6Addr) {
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_len = sizeof(sin6);
    sin6.sin6_port = htons(config.serverPort);
    if (!inet_pton(AF_INET6, serverIpv6Addr.UTF8String, &sin6.sin6_addr.s6_addr)) {
      return nil;
    }

    config.serverIpv6Addr = nw_endpoint_create_address((struct sockaddr*)&sin6);
  }

  // Parse the allowed IP address ranges.
  NSObject* rangesObject = [config.dict objectForKey:@"routes"];
  NSMutableArray<RoutePrefix*>* routes = [NSMutableArray new];
  if ([rangesObject isKindOfClass:[NSArray<NSString*> class]]) {
    NSArray<NSString*>* list = (NSArray<NSString*>*)rangesObject;
    for (NSString* rangeString in list) {
      NSArray<NSString*>* split = [rangeString componentsSeparatedByString:@"/"];
      NSString* dest = split[0];
      RoutePrefix* prefix = [RoutePrefix new];

      if ([dest containsString:@":"]) {
        // IPv6 address
        struct sockaddr_in6 sin6;
        memset(&sin6, 0, sizeof(sin6));
        sin6.sin6_family = AF_INET6;
        sin6.sin6_len = sizeof(sin6);
        sin6.sin6_port = 0; // Not used.
        if (!inet_pton(AF_INET6, dest.UTF8String, &sin6.sin6_addr.s6_addr)) {
          return nil;
        }
        prefix.destination = nw_endpoint_create_address((struct sockaddr*)&sin6);
        prefix.prefixLength = 128;
      } else {
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_len = sizeof(sin);
        sin.sin_port = 0; // Not used.
        if (!inet_pton(AF_INET, dest.UTF8String, &sin.sin_addr.s_addr)) {
          return nil;
        }
        prefix.destination = nw_endpoint_create_address((struct sockaddr*)&sin);
        prefix.prefixLength = 32;
      }

      if (split.count > 1) {
        NSInteger i = split[1].integerValue;
        if ((i < 0) || (i > prefix.prefixLength)) {
          return nil;
        }
        prefix.prefixLength = i;
      }

      [routes addObject:prefix];
    }
  }
  config.routes = [NSArray arrayWithArray:routes];

  return config;
}

@end
