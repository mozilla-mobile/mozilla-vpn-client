/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Cocoa/Cocoa.h>

int main()
{
    NSString *bundleIdentifier = [NSBundle.mainBundle objectForInfoDictionaryKey:@"org.mozilla.macos.FirefoxVPN"];

    NSAppleEventDescriptor *paramDescriptor = [NSAppleEventDescriptor nullDescriptor];

    [NSWorkspace.sharedWorkspace launchAppWithBundleIdentifier:bundleIdentifier
                                                       options:NSWorkspaceLaunchWithoutActivation
                                additionalEventParamDescriptor:paramDescriptor
                                              launchIdentifier:NULL];

    return 0;
}

