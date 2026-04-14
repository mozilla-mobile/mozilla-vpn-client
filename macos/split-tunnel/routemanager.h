/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>

struct sockaddr;

@protocol RouteManagerDelegate
- (void)defaultRouteChanged:(int)family
               viaInterface:(nw_interface_t)interface
                withGateway:(NSData*)gateway;
@end

@interface RouteManager : NSObject
- (id)init;

- (void)startWithDelegate:(NSObject<RouteManagerDelegate>*)delegate;

- (void)rtmSendRoute:(int)action
       toDestination:(NSData*)dst
          withPrefix:(unsigned int)plen
        viaInterface:(unsigned int)ifindex
         withGateway:(NSData*)gateway
            andFlags:(int)flags;
@end
