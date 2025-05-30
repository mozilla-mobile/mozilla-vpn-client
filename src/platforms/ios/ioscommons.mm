/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioscommons.h"
#include "constants.h"
#include "logger.h"
#include "loghandler.h"
#include "qmlengineholder.h"

#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QGuiApplication>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace {
Logger logger("IOSCommons");
}

// static
int IOSCommons::compareStrings(const QString& a, const QString& b) {
  NSString* aNS = a.toNSString();
  NSString* bNS = b.toNSString();
  NSComparisonResult result = [aNS compare:bNS];
  switch (result) {
    case NSOrderedAscending:
      return -1;
    case NSOrderedDescending:
      return 1;
    case NSOrderedSame:
      return 0;
    default:
      Q_ASSERT(false);
      return -1;
  }
}

// static
QStringList IOSCommons::systemLanguageCodes() {
  NSArray<NSString*>* languages = [NSLocale preferredLanguages];

  QStringList codes;
  for (NSString* language in languages) {
    codes.append(QString::fromNSString(language));
  }
  return codes;
}

@interface StatusBarModifierViewController : UIViewController
@property(nonatomic, assign) UIStatusBarStyle preferredStatusBarStyle;
@end

void IOSCommons::setStatusBarTextColor(Theme::StatusBarTextColor color) {
  StatusBarModifierViewController* rootViewController =
      static_cast<StatusBarModifierViewController*>(
          [[UIApplication sharedApplication].windows[0] rootViewController]);
  if (color == Theme::StatusBarTextColorLight) {
    rootViewController.preferredStatusBarStyle = UIStatusBarStyleLightContent;
  } else {
    rootViewController.preferredStatusBarStyle = UIStatusBarStyleDarkContent;
  }
  [rootViewController setNeedsStatusBarAppearanceUpdate];
}

// There is a bug for iOS 16 and below where the status bar text is the wrong color
// when app is restored from background. This fixes the bug. More info: VPN-2387
// Remove this code when the app's minimum supported iOS version is iOS 17.
void IOSCommons::statusBarUpdateHack() {
  if (@available(iOS 17, *)) {
    // No need to update status bar color, as works as expected on iOS 17+.
  } else {
    setStatusBarTextColor(Theme::StatusBarTextColorLight);
  }
}

void IOSCommons::shareLogs(const QString& logs) {
  UIView* view =
      static_cast<UIView*>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow(
          "uiview", QmlEngineHolder::instance()->window()));
  UIViewController* qtController = [[view window] rootViewController];

  NSURL* url =
      [NSURL fileURLWithPath:[NSTemporaryDirectory()
                                 stringByAppendingString:QString(Constants::LOG_FILE_NAME)
                                                             .toNSString()]];
  NSData* data = [logs.toNSString() dataUsingEncoding:NSUTF8StringEncoding];
  [data writeToURL:url atomically:NO];

  UIActivityViewController* activityViewController =
      [[UIActivityViewController alloc] initWithActivityItems:@[ url ] applicationActivities:nil];
  if (activityViewController.popoverPresentationController) {
    activityViewController.popoverPresentationController.sourceView = view;
    activityViewController.popoverPresentationController.sourceRect =
        CGRectMake(view.bounds.size.width / 2, view.bounds.size.height, 0, 0);
  }
  activityViewController.completionWithItemsHandler =
      ^(NSString* activityType, BOOL completed, NSArray* returnedItems, NSError* activityError) {
        if (completed) {
          logger.info() << "Clearing logs.";
          LogHandler::instance()->flushLogs();
        } else {
          logger.info() << "No need to clear logs.";
        }
      };
  [qtController presentViewController:activityViewController animated:YES completion:nil];
  [activityViewController release];
}
