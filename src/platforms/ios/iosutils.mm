/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosutils.h"

#import <UIKit/UIKit.h>

// static
QString IOSUtils::computerName()
{
    NSString *name = [[UIDevice currentDevice] name];
    return QString::fromNSString(name);
}
