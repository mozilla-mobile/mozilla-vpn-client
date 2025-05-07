/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <NetworkExtension/NetworkExtension.h>

@interface VPNSplitTunnelProvider : NETransparentProxyProvider
@end

@implementation VPNSplitTunnelProvider

- (id)init{
  self = [super init];
  NSLog(@"init proxy class");
  return self;
}

- (void)startProxyWithOptions:(NSDictionary<NSString *,id> *)options
            completionHandler:(void (^)(NSError *error))completionHandler {
    NSLog(@"starting proxy");
    completionHandler(nil);
    // TODO: Implement Me!
}
- (void)stopProxyWithReason:(NEProviderStopReason)reason 
          completionHandler:(void (^)(void))completionHandler {
    NSLog(@"stopping proxy");
    completionHandler();
    // TODO: Implement Me!
}

- (BOOL)handleNewFlow: (NEAppProxyFlow*) flow {
    // TODO: Implement Me!
    return false;
}

- (void)cancelProxyWithError:(NSError *) error {
    // TODO: Implement Me!
    NSLog(@"cancel proxy: %@", error.localizedDescription);
}

@end
