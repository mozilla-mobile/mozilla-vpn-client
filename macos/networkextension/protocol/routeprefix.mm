/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "routeprefix.h"

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

- (BOOL)isEqual:(id)obj {
  if (![obj isKindOfClass:RoutePrefix.class]) return FALSE;
  RoutePrefix* other = (RoutePrefix*)obj;

  if (self.prefixLength != other.prefixLength) return FALSE;
  if (m_storage.ss_family != other.destination->sa_family) return FALSE;

  if (m_storage.ss_family == AF_INET) {
    const struct sockaddr_in* a = (struct sockaddr_in*)&m_storage;
    const struct sockaddr_in* b = (struct sockaddr_in*)other.destination;
    return a->sin_addr.s_addr == b->sin_addr.s_addr;
  }

  if (m_storage.ss_family == AF_INET6) {
    const struct sockaddr_in6* a = (struct sockaddr_in6*)&m_storage;
    const struct sockaddr_in6* b = (struct sockaddr_in6*)other.destination;
    return memcmp(a->sin6_addr.s6_addr, b->sin6_addr.s6_addr, 16);
  }

  // TODO: We don't recognize this address family, assume non-equal?
  return FALSE;
}

- (NSUInteger)hash {
  NSUInteger seed = m_storage.ss_family + self.prefixLength;

  if (m_storage.ss_family == AF_INET) {
    const struct sockaddr_in* sin = (struct sockaddr_in*)&m_storage;
    seed ^= sin->sin_addr.s_addr;
  } else if (m_storage.ss_family == AF_INET6) {
    const struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&m_storage;
    const uint32_t* s6raw = (uint32_t *)sin6->sin6_addr.s6_addr;
    seed ^= s6raw[0] ^ s6raw[1] ^ s6raw[2] ^ s6raw[3];
  }

  return seed;
}

- (id) copyWithZone:(NSZone *) zone {
  RoutePrefix* prefix = [RoutePrefix new];
  memcpy(&prefix->m_storage, &m_storage, sizeof(m_storage));
  prefix->_prefixLength = self->_prefixLength;
  return prefix;
}

@end