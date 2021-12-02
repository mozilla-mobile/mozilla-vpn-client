/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosutils.h"
#include "logger.h"
#include "qmlengineholder.h"

#include <QDateTime>
#include <QString>
#include <QtGui>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtQuick>

#import <UIKit/UIKit.h>

namespace {
Logger logger(LOG_IOS, "IOSUtils");
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
  UIView *view = static_cast<UIView *>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("uiview", QmlEngineHolder::instance().window()));
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
