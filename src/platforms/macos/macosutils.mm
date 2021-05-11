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
  logger.log() << "Enabling login-item";

  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  NSString* loginItemAppId =
      QString("%1.login-item").arg(QString::fromNSString(appId)).toNSString();
  CFStringRef cfs = (__bridge CFStringRef)loginItemAppId;

  Boolean ok = SMLoginItemSetEnabled(cfs, startAtBoot ? YES : NO);
  logger.log() << "Result: " << ok;
}

namespace {

bool dockClickHandler(id self, SEL cmd, ...) {
  Q_UNUSED(self);
  Q_UNUSED(cmd);

  logger.log() << "Dock icon clicked.";
  QmlEngineHolder::instance()->showWindow();
  return FALSE;
}

}  // namespace

// static
void MacOSUtils::setDockClickHandler() {
  NSApplication* app = [NSApplication sharedApplication];
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
    if (!class_replaceMethod(delegateClass, shouldHandle, (IMP)dockClickHandler, "B@:")) {
      logger.log() << "Failed to replace the dock click handler";
    }
  } else if (!class_addMethod(delegateClass, shouldHandle, (IMP)dockClickHandler, "B@:")) {
    logger.log() << "Failed to register the dock click handler";
  }
}

void MacOSUtils::hideDockIcon() {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

void MacOSUtils::showDockIcon() {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

void MacOSUtils::installerStrings() {
  //% "Mozilla VPN for MacOS"
  qtTrId("macosinstaller.title");

  //% "Unable to install"
  qtTrId("macosinstaller.unsupported_version.title");

  //% "Mozilla VPN requires Mac OS X 10.6 or later."
  qtTrId("macosinstaller.unsupported_version.message");

  //% "Previous Installation Detected"
  qtTrId("macosinstaller.previous_build.title");

  //% "A previous installation of Mozilla VPN exists at /Applications/Mozilla VPN.app. This
  // installer will remove the previous installation prior to installing. Please back up any data
  // before proceeding."
  qtTrId("macosinstaller.previous_build.message");

  //% "You will now be guided through the installation steps for the Mozilla VPN. Thank you for
  // choosing your VPN from the trusted pioneer of internet privacy."
  qtTrId("macosinstaller.welcome.message1");

  //% "Click \"Continue\" to continue the setup."
  qtTrId("macosinstaller.welcome.message2");

  //% "The Mozilla VPN is successfully installed. Go to your Applications folder to open up the VPN
  // and start taking control of your online privacy."
  qtTrId("macosinstaller.conclusion.message1");

  //% "Trouble with this install?"
  qtTrId("macosinstaller.conclusion.message2");

  //% "Get help."
  qtTrId("macosinstaller.conclusion.message3");
}
