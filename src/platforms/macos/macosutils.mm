/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"

#include "feature/features.h"
#include "logger.h"
#include "qmlengineholder.h"

#include <QGlobalStatic>

#include <objc/message.h>
#include <objc/objc.h>

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger("MacOSUtils");
}

Q_GLOBAL_STATIC(MacOSUtils, macosUtils);
MacOSUtils* MacOSUtils::instance() { return macosUtils; }

// static
QString MacOSUtils::appId(const QString& suffix) {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (!appId) {
    // Fallback. When an unsigned/un-notarized app is executed in
    // command-line mode, it could fail the fetching of its own bundle id.
    appId = @"org.mozilla.macos.FirefoxVPN";
  }

  return QString::fromNSString(appId) + suffix;
}

void MacOSUtils::openSystemSettingsLink() {
  if (!Feature::isEnabled(Feature::networkExtension)) {
    // When using a daemon installed via the SMAppService API, there is a
    // helper method to go directly to the appropriate settings screen.
    [SMAppService openSystemSettingsLoginItems];
  } else if (@available(macOS 15.0, *)) {
    // Users on macOS 15 and later: System extensions are managed via a special
    // section in the Login Items panel. We can navigate directly to the
    // extensions section and, from there, the user must find the Mozilla VPN
    // network extension and enable it.
    NSString* url = @"x-apple.systempreferences:com.apple.LoginItems-Settings.extension?ExtensionItems";
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
  } else {
    // Users on macOS 13/14: When the system extension is blocked, a message
    // along the lines of "System software from application 'Mozilla VPN' was
    // blocked from loading" will be displayed in the Security panel. The users
    // must click "Allow" on this screen to install the system extension.
    NSString* url = @"x-apple.systempreferences:com.apple.preference.security?Security";
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
  }
}

// static
int MacOSUtils::getMacOSMajorVersion() {
  NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
  return version.majorVersion;
}

// static
QString MacOSUtils::computerName() {
  NSString* name = [[NSHost currentHost] localizedName];
  return QString::fromNSString(name);
}

// static
void MacOSUtils::enableLoginItem(bool startAtBoot) {
  logger.debug() << "Enabling login-item";

  NSError* error = nil;

  // Use register() or unregister() based on the startAtBoot flag
  if (startAtBoot) {
    if (![[SMAppService mainAppService] registerAndReturnError: & error]) {
      logger.error() << "Failed to register Mozilla VPN LoginItem: " << error;
    } else {
      logger.debug() << "Mozilla VPN LoginItem registered successfully.";
    }
  } else {
    [[SMAppService mainAppService] unregisterWithCompletionHandler:^(NSError* error){
      if (error != nil) {
        logger.warning() << "Failed to unregister Mozilla VPN LoginItem:" << error;
      } else {
        logger.debug() << "LoginItem unregistered successfully.";
      }
    }];
  }
}

namespace {

bool dockClickHandler(id self, SEL cmd, ...) {
  Q_UNUSED(self);
  Q_UNUSED(cmd);

  logger.debug() << "Dock icon clicked.";
  QmlEngineHolder::instance()->showWindow();
  return FALSE;
}

}  // namespace

// static
void MacOSUtils::setDockClickHandler() {
  NSApplication* app = [NSApplication sharedApplication];
  if (!app) {
    logger.debug() << "No sharedApplication";
    return;
  }

  id delegate = [app delegate];
  if (!delegate) {
    logger.debug() << "No delegate";
    return;
  }

  Class delegateClass = [delegate class];
  if (!delegateClass) {
    logger.debug() << "No delegate class";
    return;
  }

  SEL shouldHandle = sel_registerName("applicationShouldHandleReopen:hasVisibleWindows:");
  if (class_getInstanceMethod(delegateClass, shouldHandle)) {
    if (!class_replaceMethod(delegateClass, shouldHandle, (IMP)dockClickHandler, "B@:")) {
      logger.error() << "Failed to replace the dock click handler";
    }
  } else if (!class_addMethod(delegateClass, shouldHandle, (IMP)dockClickHandler, "B@:")) {
    logger.error() << "Failed to register the dock click handler";
  }
}

void MacOSUtils::hideDockIcon() {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

void MacOSUtils::showDockIcon() {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

/**
 * Replace the setImage method on NSStatusBarButton with a method that scales
 * images proportionally before setting.
 *
 * The reason for this is that there is a bug in Qt 5.15 that causes status bar
 * icons to be displayed larger than UI recommendations, and out of proportion
 * on displays with a device pixel ratio greater than 1 (MacOS Big Sur only).
 * This bug will not be fixed in Qt open source versions, so we have to resort
 * to a hack that exchanges the implementation of a method on NSStatusBarButton
 * with one that correctly scales the icon.
 *
 * Original bug (and sample implementation):
 * https://bugreports.qt.io/browse/QTBUG-88600
 */
void MacOSUtils::patchNSStatusBarSetImageForBigSur() {
  Method original = class_getInstanceMethod([NSStatusBarButton class], @selector(setImage:));
  Method patched = class_getInstanceMethod([NSStatusBarButton class], @selector(setImagePatched:));
  method_exchangeImplementations(original, patched);
}

@interface NSImageScalingHelper : NSObject
/**
 * Create a proportionally scaled image according to the given target size.
 *
 * @param sourceImage The original image to be scaled.
 * @param targetSize The required size of the image.
 * @return A scaled image.
 */
+ (NSImage*)imageByScaling:(NSImage*)sourceImage size:(NSSize)targetSize;
@end

@implementation NSImageScalingHelper
+ (NSImage*)imageByScaling:(NSImage*)sourceImage size:(NSSize)targetSize {
  NSImage* newImage = nil;

  if ([sourceImage isValid]) {
    NSSize sourceSize = [sourceImage size];

    if (sourceSize.width != 0.0 && sourceSize.height != 0.0) {
      float scaleFactor = 0.0;
      float scaledWidth = targetSize.width;
      float scaledHeight = targetSize.height;

      NSPoint thumbnailPoint = NSZeroPoint;

      if (NSEqualSizes(sourceSize, targetSize) == NO) {
        float widthFactor = targetSize.width / sourceSize.width;
        float heightFactor = targetSize.height / sourceSize.height;

        if (widthFactor < heightFactor) {
          scaleFactor = widthFactor;
        } else {
          scaleFactor = heightFactor;
        }
        scaledWidth = sourceSize.width * scaleFactor;
        scaledHeight = sourceSize.height * scaleFactor;

        if (widthFactor < heightFactor) {
          thumbnailPoint.y = (targetSize.height - scaledHeight) * 0.5;
        } else {
          thumbnailPoint.x = (targetSize.width - scaledWidth) * 0.5;
        }
      }

      newImage = [[NSImage alloc] initWithSize:targetSize];

      [newImage lockFocus];

      NSRect thumbnailRect;
      thumbnailRect.origin = thumbnailPoint;
      thumbnailRect.size.width = scaledWidth;
      thumbnailRect.size.height = scaledHeight;
      [sourceImage drawInRect:thumbnailRect
                     fromRect:NSZeroRect
                    operation:NSCompositingOperationSourceOver
                     fraction:1.0];

      [newImage unlockFocus];

      [newImage setTemplate:[sourceImage isTemplate]];
    }
  }
  return [newImage autorelease];
}
@end

@implementation NSStatusBarButton (Swizzle)
- (void)setImagePatched:(NSImage*)image {
  NSImage* img = image;

  if (image != nil) {
    int thickness = [[NSStatusBar systemStatusBar] thickness];
    img = [NSImageScalingHelper imageByScaling:image size:NSMakeSize(thickness, thickness)];
  }

  [self setImagePatched:img];
}
@end
