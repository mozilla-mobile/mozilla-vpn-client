/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Cocoa/Cocoa.h>

int main()
{
  if (@available(macOS 10.15, *)) {
    // When an app is sandboxed, the configuration.arguments array is ignored. Let's use env
    // variables to pass startup flags.
    NSDictionary* env = @{@"MVPN_MINIMIZED" : @"1", @"MVPN_STARTATBOOT" : @"1"};

    NSWorkspaceOpenConfiguration* configuration = [NSWorkspaceOpenConfiguration new];
    [configuration setEnvironment:env];

    [[NSWorkspace sharedWorkspace]
        openApplicationAtURL:[NSURL fileURLWithPath:@"/Applications/Mozilla VPN.app"]
               configuration:configuration
           completionHandler:^(NSRunningApplication* _Nullable app, NSError* _Nullable error) {
             if (error) {
               NSLog(@"Failed to run the Mozilla VPN app: %@", error.localizedDescription);
             }
             exit(0);
           }];

    [NSThread sleepForTimeInterval:10];
  } else {
    NSString* appId = [NSString stringWithUTF8String:APP_ID];

    [[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:appId
                                                         options:NSWorkspaceLaunchDefault
                                  additionalEventParamDescriptor:NULL
                                                launchIdentifier:NULL];
  }

  return 0;
}

