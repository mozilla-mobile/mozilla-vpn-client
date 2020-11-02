/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"
#include "logger.h"

#include <objc/message.h>
#include <objc/objc.h>
#include <QApplication>
#include <QWindow>

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger(LOG_MACOS, "MacOSUtils");
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

namespace {
bool dockClickHandler(id self, SEL cmd, ...)
{
    Q_UNUSED(self);
    Q_UNUSED(cmd);

    logger.log() << "Dock icon clicked.";

    for (QWindow *window : QApplication::topLevelWindows()) {
        window->show();
    }

    return FALSE;
}

} // namespace

// static
void MacOSUtils::setDockClickHandler()
{
    NSApplication *app = [NSApplication sharedApplication];
    if (!app) {
        logger.log() << "No sharedApplication";
        return;
    }

    id delegate = [app delegate];
    if (!delegate) {
        logger.log() << "No delegate";
        return;
    }

    Class delegateClass = [delegate class];
    if (!delegateClass) {
        logger.log() << "No delegate class";
        return;
    }

    SEL shouldHandle = sel_registerName("applicationShouldHandleReopen:hasVisibleWindows:");
    if (class_getInstanceMethod(delegateClass, shouldHandle)) {
        if (!class_replaceMethod(delegateClass, shouldHandle, (IMP) dockClickHandler, "B@:")) {
            logger.log() << "Failed to replace the dock click handler";
        }
    } else if (!class_addMethod(delegateClass, shouldHandle, (IMP) dockClickHandler, "B@:")) {
        logger.log() << "Failed to register the dock click handler";
    }
}
