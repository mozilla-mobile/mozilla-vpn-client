/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@interface BypassTcpFlow : NSObject

+ (id)createBypass:(NEAppProxyTCPFlow*)flow
        toEndpoint:(nw_endpoint_t)endpoint
     withInterface:(nw_interface_t)interface;

- (void)startBypass:(void (^)(NSError* error))completionHandler;

@property(strong) NEAppProxyTCPFlow* flow;
@property(strong) nw_connection_t connection;

@end
