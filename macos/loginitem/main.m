/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Cocoa/Cocoa.h>

int main()
{
    NSString *appId = [NSString stringWithUTF8String: APP_ID];
    fprintf(stderr, "BAKU BAKU %s\n", [appId UTF8String]);

    

    [[NSWorkspace sharedWorkspace]
           launchAppWithBundleIdentifier:appId
                                 options:NSWorkspaceLaunchDefault
          additionalEventParamDescriptor:NULL
                        launchIdentifier:NULL];

    return 0;
}

