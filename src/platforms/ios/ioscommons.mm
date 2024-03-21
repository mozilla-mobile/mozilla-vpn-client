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

// static
bool IOSCommons::verifySignature(const QByteArray& publicKey, const QByteArray& content,
                                 const QByteArray& signature) {
  NSDictionary* attributes = @{
    (__bridge NSString*)kSecAttrKeyType : (__bridge NSString*)kSecAttrKeyTypeRSA,
    (__bridge NSString*)kSecAttrKeyClass : (__bridge NSString*)kSecAttrKeyClassPublic
  };
  SecKeyRef publicKeyRef = SecKeyCreateWithData((CFDataRef)publicKey.toNSData(),
                                                (__bridge CFDictionaryRef)attributes, nullptr);

  if (SecKeyVerifySignature(publicKeyRef, kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA256,
                            (CFDataRef)content.toNSData(), (CFDataRef)signature.toNSData(),
                            nullptr)) {
    return true;
  }

  logger.warning() << "Signature verification failed";
  return false;
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
