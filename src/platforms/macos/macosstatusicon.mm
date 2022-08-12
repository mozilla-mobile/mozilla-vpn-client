/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosstatusicon.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#import <Cocoa/Cocoa.h>

@interface MacOSStatusIconDelegate : NSObject
@property (atomic, assign, readwrite) NSStatusItem* statusItem;
@property (atomic, assign, readwrite) NSView* statusIndicator;

- (id)init;
- (void)setIcon:(NSString*)iconUrl;
- (void)setIndicator;
- (void)setIndicatorColor:(NSColor*)color;
- (void)setMenu:(NSMenu*)statusBarMenu;
@end

@implementation MacOSStatusIconDelegate
- (id)init {
  self = [super init];

  // Create status item
  self.statusItem = [[[NSStatusBar systemStatusBar]
      statusItemWithLength:NSSquareStatusItemLength] retain];
  self.statusItem.visible = true;

  // Add the indicator as a subview
  [self setIndicator];

  return self;
}

- (void)setIcon:(NSString*)iconUrl {
  NSImage* image = [[NSImage alloc] initWithContentsOfFile:iconUrl];
  [image setTemplate:true];

  [self.statusItem.button setImage:image];
}

- (void)setIndicator {
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
  self.statusIndicator.layer.backgroundColor = color.CGColor;
}

- (void)setMenu:(NSMenu*)statusBarMenu {
  [self.statusItem setMenu:statusBarMenu];
}
@end

namespace {
Logger logger(LOG_MACOS, "MacOSStatusIcon");

MacOSStatusIconDelegate* m_statusBarIcon = nullptr;
}

MacOSStatusIcon::MacOSStatusIcon(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(MacOSStatusIcon);

  if (m_statusBarIcon) {
    logger.debug() << "Delegate already registered";
    return;
  }

  logger.debug() << "Registering delegate";
  m_statusBarIcon = [[MacOSStatusIconDelegate alloc] init];
}

MacOSStatusIcon::~MacOSStatusIcon() {
  MVPN_COUNT_DTOR(MacOSStatusIcon);

  if (m_statusBarIcon) {
    [static_cast<MacOSStatusIconDelegate*>(m_statusBarIcon) dealloc];
    m_statusBarIcon = nullptr;
  }
}

void MacOSStatusIcon::setStatusBarIcon(QString iconUrl) {
  logger.debug() << "Set status bar icon" << iconUrl;
  [m_statusBarIcon setIcon:iconUrl.toNSString()];
}

void MacOSStatusIcon::setStatusBarIndicatorColor(
    QColor indicatorColor) {
  logger.debug() << "Set status bar indicator color";

  if (!indicatorColor.isValid()) {
    [m_statusBarIcon setIndicatorColor:[NSColor clearColor]];
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
  [m_statusBarIcon setIndicatorColor:color];
}

void MacOSStatusIcon::setStatusBarMenu(NSMenu* statusBarMenu) {
  logger.debug() << "Set status bar menu";
  [m_statusBarIcon setMenu:statusBarMenu];
}
