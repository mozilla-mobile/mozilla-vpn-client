/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosutils.h"
#include "logger.h"
#include "models/helpmodel.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <objc/message.h>
#include <objc/objc.h>
#include <QMenu>

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

  NSString* appId = MacOSUtils::appId();
  Q_ASSERT(appId);

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

  if (@available(macOS 11.0, *)) {
    if (image != nil) {
      int thickness = [[NSStatusBar systemStatusBar] thickness];
      img = [NSImageScalingHelper imageByScaling:image size:NSMakeSize(thickness, thickness)];
    }
  }

  [self setImagePatched:img];
}
@end

@interface StatusBarIndicatorIcon : NSObject
@property (atomic, assign, readwrite) NSStatusItem* statusItem;
@property (atomic, assign, readwrite) NSView* statusIndicator;

- (id)initWithIcon:(NSString*)iconUrl;
- (void)setIcon:(NSString*)iconUrl;
- (void)setIndicator;
- (void)setIndicatorColor:(NSColor*)color;
- (void)setMenu:(NSMenu*)statusBarMenu;
@end

@implementation StatusBarIndicatorIcon
- (id)initWithIcon:(NSString*)iconUrl {
  self = [super init];

  // Create status item
  self.statusItem = [[[NSStatusBar systemStatusBar]
      statusItemWithLength:NSSquareStatusItemLength] retain];
  self.statusItem.visible = true;

  // Add the indicator as a subview
  [self setIndicator];

  // Init icon
  [self setIcon:iconUrl];

  return self;
}

- (void)setIcon:(NSString*)iconUrl {
  logger.debug() << "Set icon" << iconUrl;

  NSImage* image = [[NSImage alloc] initWithContentsOfFile:iconUrl];
  [image setTemplate:true];

  [self.statusItem.button setImage:image];
}

- (void)setIndicator {
  logger.debug() << "Set indicator";

  // Add a view to status item
  float viewHeight = NSHeight([self.statusItem.button bounds]);
  float dotSize = viewHeight * 0.35;
  float dotOrigin = (viewHeight - dotSize) * 0.8;

  self.statusIndicator = [[NSView alloc]
      initWithFrame:NSMakeRect(dotOrigin, dotOrigin, dotSize, dotSize)];
  self.statusIndicator.wantsLayer = true;
  self.statusIndicator.layer.cornerRadius = dotSize * 0.5;

  [self.statusItem.button addSubview:self.statusIndicator];
}

- (void)setIndicatorColor:(NSColor*)color {
  logger.debug() << "Set indicator color";
  self.statusIndicator.layer.backgroundColor = color.CGColor;
}

- (void)setMenu:(NSMenu*)statusBarMenu {
  logger.debug() << "Set menu";
  [self.statusItem setMenu:statusBarMenu];
}
@end

StatusBarIndicatorIcon* statusBarIcon = nil;
void MacOSUtils::setStatusBarIcon(QString iconUrl) {
  logger.debug() << "Set status bar icon";

  NSString* iconUrlNS = iconUrl.toNSString();
  if (!statusBarIcon) {
    statusBarIcon = [[StatusBarIndicatorIcon alloc]
        initWithIcon:iconUrlNS];
    return;
  }

  [statusBarIcon setIcon:iconUrlNS];
}

void MacOSUtils::setStatusBarIndicatorColor(QColor indicatorColor) {
  logger.debug() << "Set status bar indicator color";

  if (!indicatorColor.isValid()) {
    [statusBarIcon setIndicatorColor:[NSColor clearColor]];
    return;
  }

  int redValue = indicatorColor.red();
  int greenValue = indicatorColor.green();
  int blueValue = indicatorColor.blue();
  int alphaValue = indicatorColor.alpha();

  NSColor* color = [NSColor colorWithCalibratedRed:(redValue / 255)
      green:(greenValue / 255)
      blue:(blueValue / 255)
      alpha:(alphaValue / 255)];
  [statusBarIcon setIndicatorColor:color];
}

void MacOSUtils::setStatusBarMenu(NSMenu* statusBarMenu) {
  logger.debug() << "Set status bar menu";
  [statusBarIcon setMenu:statusBarMenu];
}

// static
NSString* MacOSUtils::appId() {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (!appId) {
    // Fallback. When an unsigned/un-notarized app is executed in
    // command-line mode, it could fail the fetching of its own bundle id.
    appId = @"org.mozilla.macos.FirefoxVPN";
  }

  return appId;
}
