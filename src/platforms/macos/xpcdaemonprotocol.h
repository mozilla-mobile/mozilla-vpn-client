/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCDAEMONPROTOCOL_H
#define XPCDAEMONPROTOCOL_H

#import <Foundation/Foundation.h>

@protocol XpcDaemonProtocol
- (void) activate: (NSString*)config;
- (void) deactivate;
- (void) getVersion:(void (^)(NSString*))reply;
- (void) getStatus:(void (^)(NSString*))reply;
- (void) getBackendLogs:(void (^)(NSString*))reply;
- (void) cleanupBackendLogs;
@end

@protocol XpcClientProtocol
- (void) connected: (NSString*)pubkey;
- (void) disconnected;
@end

#endif  // XPCDAEMONPROTOCOL_H
