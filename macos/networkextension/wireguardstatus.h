/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>

@interface WireguardStatus : NSObject

@property (strong) NSDate*    lastHandshake;
@property (strong) NSString*  ipv4address;
@property (strong) NSString*  ipv6address;
@property (strong) NSString*  ipv4gateway;
@property (strong) NSString*  ipv6gateway;
@property (assign) NSUInteger txBytes;
@property (assign) NSUInteger rxBytes;
@property (assign) float      estimatedLoss;
@property (assign) NSUInteger estimatedRtt;

- (void)encodeWithCoder:(NSCoder *)coder;
@end
