/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"

#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

// static
QString MacOSUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}

// static
void MacOSUtils::enableLoginItem()
{
    qDebug() << "Enabling login-item";
    Boolean ok = SMLoginItemSetEnabled(CFSTR("org.mozilla.macos.FirefoxVPN.login-item"), YES);
    qDebug() << "Result: " << ok;
}
