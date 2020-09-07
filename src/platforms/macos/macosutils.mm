#include "macosutils.h"
#import <Cocoa/Cocoa.h>

QString MacosUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}
