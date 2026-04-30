/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>

#ifndef UTILS_H

typedef enum {
    kVPNSuccess = 0,
    kVPNErrInvalidConfig = 1,
    kVPNErrTunnelNotRunning = 2,
} VPNErrorType;

nw_endpoint_t convertEndpoint(NWEndpoint* ep);
NSUInteger getWorkerCount();
NSError* vpnProviderError(VPNErrorType err, NSString* msg);
NSError* vpnPosixError(int code, NSString* msg);

#endif  // UTILS_H
