/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <NetworkExtension/NetworkExtension.h>
#import <os/log.h>

#ifndef UTILS_H

nw_endpoint_t convertEndpoint(NWEndpoint* ep);
NSUInteger getWorkerCount();
NSError* vpnProviderError(NEProviderStopReason reason);
NSError* vpnPosixError(int code, NSString* msg);

os_log_t vpn_log_create(const char* module);

int dnsManagerMain(int argc, char* argv[]);

#endif  // UTILS_H
