/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "udp.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>

@implementation UdpTransport {
  struct sockaddr_storage m_destaddr;
}

- (id)initWithEndpoint:(nw_endpoint_t)ep {
  self = [super init];

  const struct sockaddr* dest = nw_endpoint_get_address(ep);
  if (dest->sa_len > sizeof(m_destaddr)) {
    return nil;
  }
  memcpy(&m_destaddr, dest, dest->sa_len);

  self->_sockfd = socket(dest->sa_family, SOCK_DGRAM, IPPROTO_UDP);
  if (self->_sockfd < 0) {
    return nil;
  }

  return self;
}

- (void) dealloc {
  NSLog(@"udp dealloc");
  close(self.sockfd);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

- (int)connect {
  return connect(self.sockfd, (struct sockaddr*)&m_destaddr, m_destaddr.ss_len);
}

- (void)shutdown {
  shutdown(self.sockfd, SHUT_RDWR);
}

- (void)send:(NSData*)dgram {
  send(self.sockfd, dgram.bytes, dgram.length, MSG_DONTWAIT);
}

- (int)recv:(NSMutableData*)dgram {
  int rx = read(self.sockfd, dgram.mutableBytes, dgram.length);
  if (rx > 0) {
    dgram.length = rx;
  }
  return rx;
}

- (NSUInteger)overhead {
  if (m_destaddr.ss_family == AF_INET6) {
    return sizeof(struct udphdr) + sizeof(struct ip6_hdr);
  } else {
    return sizeof(struct udphdr) + sizeof(struct ipovly);
  }
}

@end
