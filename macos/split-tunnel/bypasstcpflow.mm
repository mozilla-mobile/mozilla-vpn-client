/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

#import "bypasstcpflow.h"

@implementation BypassTcpFlow {
  nw_connection_t    m_connection;
}

+ (id)createBypass:(NEAppProxyTCPFlow *)flow
     withInterface:(nw_interface_t)interface {

  BypassTcpFlow* bypass = [BypassTcpFlow new];
  bypass.flow = flow;

  nw_endpoint_t endpoint;
  if (@available(macOS 15, *)) {
    endpoint = flow.remoteFlowEndpoint;
  } else if ([flow.remoteEndpoint isKindOfClass:[NWHostEndpoint class]]) {
    NWHostEndpoint* host = (NWHostEndpoint*)flow.remoteEndpoint;
    endpoint = nw_endpoint_create_host([host.hostname UTF8String], [host.port UTF8String]);
  } else {
    // TODO: The other option is a bonjour endpoint?
  }

  // TODO: Outbound interface selection is tricky... it doesn't seem like we can
  // use nw_parameters_prohibit_interface without putting the VPN interface into
  // a different path.
  nw_parameters_t params = nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
  //nw_parameters_prohibit_interface(params, interface);
  nw_parameters_require_interface(params, interface);

  bypass->m_connection = nw_connection_create(endpoint, params);
  nw_connection_set_queue(bypass->m_connection, dispatch_get_main_queue());

  return bypass;
}

- (void)startBypass:(void (^)(NSError *error)) completionHandler {
  nw_connection_set_state_changed_handler(m_connection, ^(nw_connection_state_t state, nw_error_t error) {
    if (error) {
      NSError *err = (NSError *)CFBridgingRelease(nw_error_copy_cf_error(error));
      completionHandler(err);
    } else if (state == nw_connection_state_cancelled || state == nw_connection_state_failed) {
      NSLog(@"bypass state closed");
      completionHandler(nil);
    } else if (state != nw_connection_state_ready) {
      NSLog(@"bypass state %d", state);
    } else if (@available(macOS 15, *)) {
      NSLog(@"bypass opening");
      [self.flow openWithLocalFlowEndpoint:nil
                         completionHandler:^(NSError* openError){
        if (openError) {
          NSLog(@"bypass open error: %@", openError);
          nw_connection_cancel(m_connection);
          completionHandler(openError);
        } else {
          NSLog(@"bypass data begin");
          [self handleOutbound:completionHandler];
          [self handleInbound:completionHandler];
        }
      }];
    } else {
      NSLog(@"bypass opening (legacy)");
      [self.flow openWithLocalEndpoint:nil
                     completionHandler:^(NSError* openError){
        if (openError) {
          NSLog(@"bypass open error: %@", openError);
          nw_connection_cancel(m_connection);
          completionHandler(openError);
        } else {
          NSLog(@"bypass data begin (legacy)");
          [self handleOutbound:completionHandler];
          [self handleInbound:completionHandler];
        }
      }];
    }
  });

  nw_connection_start(m_connection);
}

- (void)handleOutbound:(void (^)(NSError *error)) completionHandler {
  [self.flow readDataWithCompletionHandler:^(NSData * _Nullable data, NSError * _Nullable error) {
    if (error) {
      [self closeConnection:error completionHandler:completionHandler];
      return;
    }

    // If there was no data, try again.
    if (!data) {
      [self handleOutbound:completionHandler];
      return;
    }

    // Outbound data flow terminated gracefully.
    if (data.length == 0) {
      [self closeConnection:nil completionHandler:completionHandler];
      return;
    }

    // Forward the data out to the network
    dispatch_data_t chunk = dispatch_data_create(data.bytes, data.length, dispatch_get_main_queue(), DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    nw_connection_send(m_connection, chunk, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t  _Nullable sendError) {
      if (sendError) {
        NSError *err = (NSError *)CFBridgingRelease(nw_error_copy_cf_error(sendError));
        [self closeConnection:err completionHandler:completionHandler];
      } else {
        [self handleOutbound:completionHandler];
      }
    });
  }];
}

- (void)handleInbound:(void (^)(NSError *)) completionHandler {
  nw_connection_receive(m_connection, 1, UINT16_MAX, ^(dispatch_data_t data, nw_content_context_t ctx, bool completed, nw_error_t error){
    if (error){
      NSError *err = (NSError *)CFBridgingRelease(nw_error_copy_cf_error(error));
      [self closeConnection:err completionHandler:completionHandler];
      return;
    }
    if (!data) {
      [self closeConnection:nil completionHandler:completionHandler];
      return;
    }

    // Forward the data to the app proxy flow.
    const void *buffer;
    size_t length;
    dispatch_data_t __unused map = dispatch_data_create_map(data, &buffer, &length);
    NSData* chunk = [NSData dataWithBytes:buffer length:length];
    [self.flow writeData:chunk withCompletionHandler:^(NSError* recvError){
      if (recvError) {
        [self closeConnection:recvError completionHandler:completionHandler];
      } else if (completed) {
        [self closeConnection:nil completionHandler:completionHandler];
      } else {
        [self handleInbound:completionHandler];
      }
    }];
  });
}

- (void)closeConnection:(NSError *)error
      completionHandler:(void (^)(NSError *)) completionHandler {
  NSLog(@"bypass close");
  nw_connection_cancel(m_connection);
  [self.flow closeReadWithError:error];
  [self.flow closeWriteWithError:error];
  completionHandler(error);
}

@end
