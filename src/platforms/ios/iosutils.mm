/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosutils.h"
#include "logging/logger.h"
#include "qmlengineholder.h"

#include <QDateTime>
#include <QString>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace {
Logger logger("IOSUtils");
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
