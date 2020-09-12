#include "macosutils.h"

#import <Cocoa/Cocoa.h>

// static
QString MacOSUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}

