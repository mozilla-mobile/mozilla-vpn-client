/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>

@interface RoutePrefix : NSObject <NSCopying>

+ (id)parseRoute:(NSString*)routeString;

@property(readonly, getter=getDestination) const struct sockaddr* destination;
@property(readonly, getter=getString) NSString* string;
@property(readonly) NSUInteger prefixLength;
@end
