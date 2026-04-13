/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#import "bypassudpflow.h"

@implementation BypassUdpFlow {
  CFSocketRef          m_socket;
  CFRunLoopSourceRef   m_source;
}

static void udpSockCallback(CFSocketRef s, CFSocketCallBackType cbType, CFDataRef address, const void * data, void *info) {
  BypassUdpFlow* bypass = (BypassUdpFlow*)info;
  if (cbType == kCFSocketDataCallBack) {
    const struct sockaddr* sa = (const struct sockaddr*)CFDataGetBytePtr(address);
    [bypass recvDatagram:(NSData*)data
            fromEndpoint:nw_endpoint_create_address(sa)];
  } else {
    NSLog(@"udpSockCallback: unexpected type %d", (int)cbType);
  }
}

+ (id)createBypass:(NEAppProxyUDPFlow *)flow
     withInterface:(nw_interface_t)interface {
  // If the packet flow is already bound then there is nothing to do here.
  if (flow.isBound) {
    return nil;
  }

  BypassUdpFlow* bypass = [BypassUdpFlow new];
  bypass.flow = flow;

  CFSocketContext ctx = { .info = (void *)bypass };
  bypass->m_socket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP,
                                    kCFSocketDataCallBack, udpSockCallback, &ctx);

  // TODO: If flow.remoteHostname is set should we turn this into a connected socket?

  // TODO: What should we do if a local endpoint was specified?
#if 0
  nw_endpoint_t endpoint = nil;
  if (@available(macOS 15, *)) {
    endpoint = flow.localFlowEndpoint;
  } else if (flow.localEndpoint && [flow.localEndpoint isKindOfClass:[NWHostEndpoint class]]) {
    NWHostEndpoint* host = (NWHostEndpoint*)flow.localEndpoint;
    endpoint = nw_endpoint_create_host([host.hostname UTF8String], [host.port UTF8String]);
  }
#endif
  // Bind the socket to the bypass interface.
  int sockfd = CFSocketGetNative(bypass->m_socket);
  int ifindex = nw_interface_get_index(interface);
  setsockopt(sockfd, IPPROTO_IP, IP_BOUND_IF, &ifindex, sizeof(ifindex));
  setsockopt(sockfd, IPPROTO_IPV6, IPV6_BOUND_IF, &ifindex, sizeof(ifindex));

  // Create a source and attach it to the main run loop.
  bypass->m_source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, bypass->m_socket, 0);
  CFRunLoopAddSource(CFRunLoopGetMain(), bypass->m_source, kCFRunLoopDefaultMode);

  return bypass;
}

- (void)dealloc {
  [self closeConnection:nil completionHandler:nil];
  [super dealloc];
}

- (void)startBypass:(void (^)(NSError *error)) completionHandler {
  if (@available(macOS 15, *)) {
    NSLog(@"bypass opening");
    [self.flow openWithLocalFlowEndpoint:self.flow.localFlowEndpoint
                       completionHandler:^(NSError* openError){
      if (openError) {
        NSLog(@"bypass open error: %@", openError);
        //nw_connection_cancel(m_connection);
        completionHandler(openError);
      } else {
        [self handleOutbound:completionHandler];
      }
    }];
  } else if ([self.flow.localEndpoint isKindOfClass:[NWHostEndpoint class]]) {
    NSLog(@"bypass opening (legacy bound)");
    [self.flow openWithLocalEndpoint:(NWHostEndpoint*)self.flow.localEndpoint
                   completionHandler:^(NSError* openError){
      if (openError) {
        NSLog(@"bypass open error: %@", openError);
        [self closeConnection:openError completionHandler:completionHandler];
      } else {
        [self handleOutbound:completionHandler];
      }
    }];
  } else if (self.flow.localEndpoint == nil) {
    NSLog(@"bypass opening (legacy unbound)");
    [self.flow openWithLocalEndpoint:nil
                   completionHandler:^(NSError* openError){
      if (openError) {
        NSLog(@"bypass open error: %@", openError);
        [self closeConnection:openError completionHandler:completionHandler];
      } else {
        [self handleOutbound:completionHandler];
      }
    }];
  } else {
    // Otherwise, we don't support split-tunneling this endpoint type.
    // It's probably a bonjour endpoint.
    // TODO: We should probably return NO from handleNewFlow in this case.
    [self closeConnection:nil completionHandler:completionHandler];
  }
}

- (void)handleOutbound:(void (^)(NSError *error)) completionHandler {
  if (@available(macOS 15, *)) {
    [self.flow readDatagramsAndFlowEndpointsWithCompletionHandler:^(NSArray<NSData *> *datagrams, NWEndpointArray *remoteEndpoints, NSError *err) {
      if (err) {
        NSLog(@"dgram error: %@", err);
        [self closeConnection:err completionHandler:completionHandler];
        return;
      }

      // If there was no data, try again.
      if (!datagrams) {
        [self handleOutbound:completionHandler];
        return;
      }

      // Outbound data flow terminated gracefully.
      if (datagrams.count == 0) {
        [self closeConnection:nil completionHandler:completionHandler];
        return;
      }

      // Process the outbound datagrams.
      for (NSUInteger i = 0; i < datagrams.count; i++) {
        [self sendDatagram:datagrams[i] toEndpoint:remoteEndpoints[i]];
      }

      // Datagrams handled, try again to continue processing the flow.
      [self handleOutbound:completionHandler];
    }];
  } else {
    [self.flow readDatagramsWithCompletionHandler:^(NSArray<NSData *> *datagrams, NSArray<NWEndpoint*> *remoteEndpoints, NSError *err) {
      if (err) {
        [self closeConnection:err completionHandler:completionHandler];
        return;
      }

      // If there was no data, try again.
      if (!datagrams) {
        [self handleOutbound:completionHandler];
        return;
      }

      // Outbound data flow terminated gracefully.
      if (datagrams.count == 0) {
        [self closeConnection:nil completionHandler:completionHandler];
        return;
      }

      // Process the outbound datagrams.
      for (NSUInteger i = 0; i < datagrams.count; i++) {
        NWEndpoint* legacyEndpoint = remoteEndpoints[i];
        if ([legacyEndpoint isKindOfClass:[NWHostEndpoint class]]) {
          NWHostEndpoint* host = (NWHostEndpoint*)legacyEndpoint;
          nw_endpoint_t ep = nw_endpoint_create_host([host.hostname UTF8String], [host.port UTF8String]);
          [self sendDatagram:datagrams[i] toEndpoint:ep];
        }
      }

      // Datagrams handled, try again to continue processing the flow.
      [self handleOutbound:completionHandler];
    }];
  }
}

- (void)sendDatagram:(NSData*)dgram
          toEndpoint:(nw_endpoint_t)ep {
  // TODO: Handle address and URL endpoints?
  if (nw_endpoint_get_type(ep) != nw_endpoint_type_address) {
    // We cannot handle this destination address type.
    NSLog(@"dgram: unsupported type %d", (int)nw_endpoint_get_type(ep));
  } else {
    // Otherwise, it must be an address endpoint.
    const struct sockaddr* sa = nw_endpoint_get_address(ep);
    CFDataRef dst = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)sa,
                                                sa->sa_len, kCFAllocatorNull);
    CFSocketError err = CFSocketSendData(m_socket, dst, (CFDataRef)dgram, 0);
    // TODO: Do we care about the error?
    CFRelease(dst);
  }
}

- (void)recvDatagram:(NSData*)dgram
        fromEndpoint:(nw_endpoint_t)ep {
  if (@available(macOS 15, *)) {
    [self.flow writeDatagrams:[NSArray arrayWithObject:dgram]
          sentByFlowEndpoints:[NSArray arrayWithObject:ep]
            completionHandler:^(NSError* error){
      if (error) {
        // TODO: Handle the error?
      }
    }];
  } else {
    char* addr = nw_endpoint_copy_address_string(ep);
    char* port = nw_endpoint_copy_port_string(ep);
    NWHostEndpoint* host =
        [NWHostEndpoint endpointWithHostname:[NSString stringWithUTF8String:addr]
                                        port:[NSString stringWithUTF8String:port]];

    [self.flow writeDatagrams:[NSArray arrayWithObject:dgram]
              sentByEndpoints:[NSArray arrayWithObject:host]
            completionHandler:^(NSError* error){
      if (error) {
        // TODO: Handle the error?
      }
    }];

    free(addr);
    free(port);
  }
}

- (void)closeConnection:(NSError *)error
      completionHandler:(void (^)(NSError *)) completionHandler {
  NSLog(@"bypass close");
  // Close the flow.
  if (self.flow) {
    [self.flow closeReadWithError:error];
    [self.flow closeWriteWithError:error];
    self.flow = nil;
  }

  // And the associated bypass socket.
  if (m_socket) {
    CFSocketInvalidate(m_socket);
    CFRelease(m_socket);
    m_socket = nil;
  }
  if (m_source) {
    CFRunLoopRemoveSource(CFRunLoopGetMain(), m_source, kCFRunLoopDefaultMode);
    CFRelease(m_source);
    m_source = nil;
  }

  if (completionHandler) {
    completionHandler(error);
  }
}

@end
