/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosutils.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "theme.h"

#include <QDateTime>
#include <QString>
#include <QtGui>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtQuick>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace {
Logger logger(LOG_IOS, "IOSUtils");
}

// static
NSString* IOSUtils::appId() {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (!appId) {
    // Fallback. When an unsigned/un-notarized app is executed in
    // command-line mode, it could fail the fetching of its own bundle id.
    appId = @"org.mozilla.ios.FirefoxVPN";
  }

  return appId;
}

// static
QString IOSUtils::computerName() {
  NSString* name = [[UIDevice currentDevice] name];
  return QString::fromNSString(name);
}

// static
QString IOSUtils::IAPReceipt() {
  logger.debug() << "Retrieving IAP receipt";

  NSURL* receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
  NSData* receipt = [NSData dataWithContentsOfURL:receiptURL];

  // All the following is for debug only.
  NSString* path = [receiptURL path];
  Q_ASSERT(path);

  logger.debug() << "Receipt URL:" << QString::fromNSString(path);

  NSFileManager* fileManager = [NSFileManager defaultManager];
  Q_ASSERT(fileManager);

  NSDictionary* fileAttributes = [fileManager attributesOfItemAtPath:path error:NULL];
  if (fileAttributes) {
    NSNumber* fileSize = [fileAttributes objectForKey:NSFileSize];
    if (fileSize) {
      logger.debug() << "File size:" << [fileSize unsignedLongLongValue];
    }

    NSString* fileOwner = [fileAttributes objectForKey:NSFileOwnerAccountName];
    if (fileOwner) {
      logger.debug() << "Owner:" << QString::fromNSString(fileOwner);
    }

    NSDate* fileModDate = [fileAttributes objectForKey:NSFileModificationDate];
    if (fileModDate) {
      logger.debug() << "Modification date:" << QDateTime::fromNSDate(fileModDate).toString();
    }
  }

  if (!receipt) {
    return QString();
  }

  NSString* encodedReceipt = [receipt base64EncodedStringWithOptions:0];
  return QString::fromNSString(encodedReceipt);
}

void IOSUtils::shareLogs(const QString& logs) {
  UIView *view = static_cast<UIView *>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("uiview", QmlEngineHolder::instance()->window()));
  UIViewController *qtController = [[view window] rootViewController];

  NSURL *url = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingString:@"MozillaVPN-logs.txt"]];
  NSData *data = [logs.toNSString() dataUsingEncoding:NSUTF8StringEncoding];
  [data writeToURL:url atomically:NO];

  UIActivityViewController *activityViewController = [[UIActivityViewController alloc] initWithActivityItems:@[url] applicationActivities:nil];
  if (activityViewController.popoverPresentationController) {
    activityViewController.popoverPresentationController.sourceView = view;
    activityViewController.popoverPresentationController.sourceRect =
        CGRectMake(view.bounds.size.width / 2, view.bounds.size.height, 0, 0);
  }
  [qtController presentViewController:activityViewController animated:YES completion:nil];
  [activityViewController release];
}

// static
int IOSUtils::compareStrings(const QString& a, const QString& b) {
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

@interface StatusBarModifierViewController : UIViewController
@property (nonatomic, assign) UIStatusBarStyle preferredStatusBarStyle;
@end

void IOSUtils::setStatusBarTextColor(Theme::StatusBarTextColor color) {
    StatusBarModifierViewController* rootViewController = static_cast<StatusBarModifierViewController *>([[UIApplication sharedApplication].windows[0] rootViewController]);
    if (color == Theme::StatusBarTextColorLight) {
        rootViewController.preferredStatusBarStyle = UIStatusBarStyleLightContent;
    } else {
        rootViewController.preferredStatusBarStyle = UIStatusBarStyleDarkContent;
    }
    [rootViewController setNeedsStatusBarAppearanceUpdate];
}

// static
bool IOSUtils::verifySignature(const QByteArray& publicKey, const QByteArray& content,
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
