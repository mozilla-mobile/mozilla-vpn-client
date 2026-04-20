/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>

@interface InterfaceConfig : NSObject

+ (id)parseFromDict:(NSDictionary<NSString *,id> *)dict;
+ (id)parseFromCoder:(NSCoder*)coder;

@property (strong) NSString* privateKey;
@property (strong) nw_endpoint_t deviceIpv4Addr;
@property (strong) nw_endpoint_t deviceIpv6Addr;

@property (strong) NSString* serverPublicKey;
@property NSUInteger serverPort;
@property (strong) nw_endpoint_t serverIpv4Addr;
@property (strong) nw_endpoint_t serverIpv6Addr;

@property (strong, readonly) NSDictionary<NSString *,id>* dict;

@end
