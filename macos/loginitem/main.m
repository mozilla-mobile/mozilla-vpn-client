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

