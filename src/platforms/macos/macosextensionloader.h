/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSEXTENSIONLOADER_H
#define MACOSEXTENSIONLOADER_H

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <SystemExtensions/SystemExtensions.h>

@interface MacosExtensionLoader : NSObject <OSSystemExtensionRequestDelegate>
@property(readonly) NSString* identifier;
@property(retain) NETransparentProxyManager* manager;
@end

#endif  // MACOSEXTENSIONLOADER_H
