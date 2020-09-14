#include "iosutils.h"

#import <UIKit/UIKit.h>

// static
QString IOSUtils::computerName()
{
    NSString *name = [[UIDevice currentDevice] name];
    return QString::fromNSString(name);
}
