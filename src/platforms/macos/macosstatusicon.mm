/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosstatusicon.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#import <Cocoa/Cocoa.h>
#import <QResource>

/**
 * Creates a NSStatusItem with that can hold an icon. Additionally a NSView is
 * set as a subview to the button item of the status item. The view serves as
 * an indicator that can be displayed in color eventhough the icon is set as a
 * template. In that way we give the system control over it’s effective
 * appearance.
 */
@interface MacOSStatusIconDelegate : NSObject
@property (assign) NSStatusItem* statusItem;
@property (assign) NSView* statusIndicator;

- (void)setIcon:(NSData*)imageData;
- (void)setIndicator;
- (void)setIndicatorColor:(NSColor*)color;
- (void)setMenu:(NSMenu*)statusBarMenu;
- (void)setToolTip:(NSString*)tooltip;
@end

@implementation MacOSStatusIconDelegate
/**
 * Initializes and sets the status item and indicator objects.
 *
 * @return An instance of MacOSStatusIconDelegate.
 */
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

/**
 * Sets the image for the status icon.
 *
 * @param iconPath The data for the icon image.
 */
- (void)setIcon:(NSData*)imageData {
  NSImage* image = [[NSImage alloc] initWithData:imageData];
  [image setTemplate:true];

  [self.statusItem.button setImage:image];
}

/**
 * Adds status indicator as a subview to the status item button.
 */
- (void)setIndicator {
  float viewHeight = NSHeight([self.statusItem.button bounds]);
  float dotSize = viewHeight * 0.35;
  float dotOrigin = (viewHeight - dotSize) * 0.8;

  self.statusIndicator = [[NSView alloc]
      initWithFrame:NSMakeRect(dotOrigin, dotOrigin, dotSize, dotSize)];
  self.statusIndicator.wantsLayer = true;
  self.statusIndicator.layer.cornerRadius = dotSize * 0.5;

  [self.statusItem.button addSubview:self.statusIndicator];
}

/**
 * Sets the color if the indicator.
 *
 * @param color The indicator background color.
 */
- (void)setIndicatorColor:(NSColor*)color {
  self.statusIndicator.layer.backgroundColor = color.CGColor;
}

/**
 * Sets the status bar menu to the status item.
 *
 * @param statusBarMenu The menu object that is passed from QT.
 */
- (void)setMenu:(NSMenu*)statusBarMenu {
  [self.statusItem setMenu:statusBarMenu];
}

/**
 * Sets the tooltip string for the status item.
 *
 * @param tooltip The tooltip string.
 */
- (void)setToolTip:(NSString*)tooltip {
  [self.statusItem.button setToolTip:tooltip];
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

void MacOSStatusIcon::setIcon(QString iconPath) {
  logger.debug() << "Set icon" << iconPath;

  QResource imageResource = QResource(iconPath);
  Q_ASSERT(imageResource.isValid());

  [m_statusBarIcon setIcon:imageResource.uncompressedData().toNSData()];
}

void MacOSStatusIcon::setIndicatorColor(
    QColor indicatorColor) {
  logger.debug() << "Set indicator color";

  if (!indicatorColor.isValid()) {
    [m_statusBarIcon setIndicatorColor:[NSColor clearColor]];
    return;
  }

  NSColor* color = [NSColor
      colorWithCalibratedRed:indicatorColor.red() / 255.0f
      green:indicatorColor.green() / 255.0f
      blue:indicatorColor.blue() / 255.0f
      alpha:indicatorColor.alpha() / 255.0f];
  [m_statusBarIcon setIndicatorColor:color];
}

void MacOSStatusIcon::setMenu(NSMenu* statusBarMenu) {
  logger.debug() << "Set menu";
  [m_statusBarIcon setMenu:statusBarMenu];
}

void MacOSStatusIcon::setToolTip(QString tooltip) {
  logger.debug() << "Set tooltip";
  [m_statusBarIcon setToolTip:tooltip.toNSString()];
}
