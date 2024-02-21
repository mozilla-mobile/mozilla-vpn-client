/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioslogger.h"
#import "MozillaVPN-Swift.h"

#include "context/constants.h"
#include "context/qmlengineholder.h"
#include "utilities/leakdetector.h"

#import <UIKit/UIKit.h>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <QString>

namespace {

// Our Swift singleton.
IOSLoggerImpl* impl = nullptr;

}  // namespace

IOSLogger::IOSLogger(const QString& tag) {
  MZ_COUNT_CTOR(IOSLogger);

  impl = [[IOSLoggerImpl alloc] initWithTag:tag.toNSString()];
  Q_ASSERT(impl);
}

IOSLogger::~IOSLogger() {
  MZ_COUNT_DTOR(IOSLogger);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}

// static
void IOSLogger::debug(const QString& message) { [impl debugWithMessage:message.toNSString()]; }

// static
void IOSLogger::info(const QString& message) { [impl infoWithMessage:message.toNSString()]; }

// static
void IOSLogger::error(const QString& message) { [impl errorWithMessage:message.toNSString()]; }

// static
void IOSLogger::shareLogs(const QString& logs) {
  UIView* view =
      static_cast<UIView*>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow(
          "uiview", QmlEngineHolder::instance()->window()));
  UIViewController* qtController = [[view window] rootViewController];

  NSURL* url = [NSURL
      fileURLWithPath:[NSTemporaryDirectory()
                          stringByAppendingString:QString(Constants::LOG_FILE_NAME).toNSString()]];
  NSData* data = [logs.toNSString() dataUsingEncoding:NSUTF8StringEncoding];
  [data writeToURL:url atomically:NO];

  UIActivityViewController* activityViewController =
      [[UIActivityViewController alloc] initWithActivityItems:@[ url ] applicationActivities:nil];
  if (activityViewController.popoverPresentationController) {
    activityViewController.popoverPresentationController.sourceView = view;
    activityViewController.popoverPresentationController.sourceRect =
        CGRectMake(view.bounds.size.width / 2, view.bounds.size.height, 0, 0);
  }
  [qtController presentViewController:activityViewController animated:YES completion:nil];
  [activityViewController release];
}
