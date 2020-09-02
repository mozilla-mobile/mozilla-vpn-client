#include "macutils.h"
#import <Cocoa/Cocoa.h>

QString MacUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}
