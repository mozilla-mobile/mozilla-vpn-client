/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioscommons.h"
#include "logger.h"

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
