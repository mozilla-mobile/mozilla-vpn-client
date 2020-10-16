/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"
#include "logger.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger("MacOSUtils");
}

// static
QString MacOSUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}

// static
void MacOSUtils::enableLoginItem(bool startAtBoot)
{
    logger.log() << "Enabling login-item";
    Boolean ok = SMLoginItemSetEnabled(CFSTR("org.mozilla.macos.FirefoxVPN.login-item"), startAtBoot ? YES : NO);
    logger.log() << "Result: " << ok;
}
