/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>
#import <os/log.h>
#import <os/signpost.h>

int main(int argc, char *argv[])
{
    @autoreleasepool {
        os_log_t logHandle = nil;
        logHandle = os_log_create("org.mozilla.macos.FirefoxVPN.split-tunnel", "Default");
        os_log_info(logHandle, "Started: %{public}@ (pid: %d / uid: %d)", NSProcessInfo.processInfo.arguments.firstObject, getpid(), getuid());

        [NEProvider startSystemExtensionMode];
    }
    
    dispatch_main();
    return 0;
}
