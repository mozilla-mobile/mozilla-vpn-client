/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"
#include "logger.h"
#include "models/helpmodel.h"
#include "qmlengineholder.h"

#include <objc/message.h>
#include <objc/objc.h>

#include <QFile>
#include <QMenuBar>

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

namespace {
Logger logger(LOG_MACOS, "MacOSUtils");
}

// static
QString MacOSUtils::computerName() {
  NSString* name = [[NSHost currentHost] localizedName];
  return QString::fromNSString(name);
}

// static
void MacOSUtils::enableLoginItem(bool startAtBoot) {
  logger.debug() << "Enabling login-item";

  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  NSString* loginItemAppId =
      QString("%1.login-item").arg(QString::fromNSString(appId)).toNSString();
  CFStringRef cfs = (__bridge CFStringRef)loginItemAppId;

  Boolean ok = SMLoginItemSetEnabled(cfs, startAtBoot ? YES : NO);
  logger.debug() << "Result: " << ok;
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

// TODO this needs a better name
void MacOSUtils::adjustMenuBarIconForBigSur() {
  Method originalMethod = class_getInstanceMethod([NSStatusBarButton class], @selector(setImage:));
  Method extendedMethod =
      class_getInstanceMethod([NSStatusBarButton class], @selector(patched_setImage:));
  method_exchangeImplementations(originalMethod, extendedMethod);
}

// TODO format, organize properly and mark up with comments
// Reference: https://bugreports.qt.io/browse/QTBUG-88600

@interface NSImageScalingHelper : NSObject
+ (NSImage*)imageByScalingProportionallyToSize:(NSImage*)sourceImage size:(NSSize)targetSize;
@end

@implementation NSImageScalingHelper
+ (NSImage*)imageByScalingProportionallyToSize:(NSImage*)sourceImage size:(NSSize)targetSize {
  NSImage* newImage = nil;

  if ([sourceImage isValid]) {
    NSSize imageSize = [sourceImage size];
    float width = imageSize.width;
    float height = imageSize.height;

    if (width != .0 || height != .0) {
      float targetWidth = targetSize.width;
      float targetHeight = targetSize.height;

      float scaleFactor = 0.0;
      float scaledWidth = targetWidth;
      float scaledHeight = targetHeight;

      NSPoint thumbnailPoint = NSZeroPoint;

      if (NSEqualSizes(imageSize, targetSize) == NO) {
        float widthFactor = targetWidth / width;
        float heightFactor = targetHeight / height;

        if (widthFactor < heightFactor) {
          scaleFactor = widthFactor;
        } else {
          scaleFactor = heightFactor;
        }
        scaledWidth = width * scaleFactor;
        scaledHeight = height * scaleFactor;

        if (widthFactor < heightFactor) {
          thumbnailPoint.y = (targetHeight - scaledHeight) * 0.5;
        } else if (widthFactor > heightFactor) {
          thumbnailPoint.x = (targetWidth - scaledWidth) * 0.5;
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
- (void)patched_setImage:(NSImage*)image {
  NSImage* img = image;

  // TODO reenable
  // if (@available(macOS 10.16, *)) {
  if (image != nil) {
    int thickness = [[NSStatusBar systemStatusBar] thickness];
    img =
        [NSImageScalingHelper imageByScalingProportionallyToSize:image
                                                            size:NSMakeSize(thickness, thickness)];
  }
  // }

  [self patched_setImage:img];
}
@end
