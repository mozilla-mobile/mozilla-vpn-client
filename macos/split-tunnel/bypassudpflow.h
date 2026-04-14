/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@interface BypassUdpFlow : NSObject

+ (id)createBypass:(NEAppProxyUDPFlow *)flow withInterface:(nw_interface_t)interface;

- (void)startBypass:(void (^)(NSError* error))completionHandler;

@property(strong) NEAppProxyUDPFlow* flow;

@end
