/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>
#import <NetworkExtension/NetworkExtension.h>

int main(int argc, char *argv[])
{
  @autoreleasepool {
    NSLog(@"started: %@ (pid: %d / uid: %d)", NSProcessInfo.processInfo.arguments.firstObject, getpid(), getuid());

    [NEProvider startSystemExtensionMode];

    [[NSRunLoop mainRunLoop] run];
  }
  return 0;
}
