/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosauthenticationlistener.h"

#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QUrlQuery>
#include <QWindow>
#include <QtGlobal>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"

#import <AuthenticationServices/ASWebAuthenticationSession.h>
#import <UIKit/UIKit.h>

namespace {

Logger logger("IOSAuthenticationListener");
ASWebAuthenticationSession* session = nullptr;

UIView* resolvePresentationView() {
  QQmlApplicationEngine* engine =
      qobject_cast<QQmlApplicationEngine*>(QmlEngineHolder::instance()->engine());

  QObject* rootObject = nullptr;
  if (engine && !engine->rootObjects().isEmpty()) {
    rootObject = engine->rootObjects().first();
  }

  QWindow* window = rootObject ? qobject_cast<QWindow*>(rootObject) : nullptr;
  if (window) {
    UIView* view = static_cast<UIView*>(
        QGuiApplication::platformNativeInterface()->nativeResourceForWindow("uiview", window));
    if (view) {
      return view;
    }
  }

  return nil;
}

}  // namespace

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
@interface ContextProvider : NSObject <ASWebAuthenticationPresentationContextProviding> {
  UIView* m_view;
}
@end

@implementation ContextProvider

- (id)initWithUIView:(UIView*)uiView {
  self = [super init];
  if (self) {
    m_view = uiView;
  }
  return self;
}

- (nonnull ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:
    (nonnull ASWebAuthenticationSession*)unusedSession {
  Q_UNUSED(unusedSession);
  return m_view.window;
}

@end
#endif

IOSAuthenticationListener::IOSAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MZ_COUNT_CTOR(IOSAuthenticationListener);
}

IOSAuthenticationListener::~IOSAuthenticationListener() {
  MZ_COUNT_DTOR(IOSAuthenticationListener);

  if (session) {
    [session cancel];
    [session dealloc];
    session = nullptr;
  }
}

void IOSAuthenticationListener::start(Task* task, const QString& codeChallenge,
                                      const QString& codeChallengeMethod,
                                      const QString& emailAddress) {
  logger.debug() << "IOSAuthenticationListener initialize";

  Q_UNUSED(task);

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod, emailAddress));
  Q_ASSERT(!session);

  NSString* callbackScheme = [NSString stringWithUTF8String:Constants::DEEP_LINK_SCHEME];
  session = [[ASWebAuthenticationSession alloc]
            initWithURL:url.toNSURL()
      callbackURLScheme:callbackScheme
      completionHandler:^(NSURL* _Nullable callbackURL, NSError* _Nullable error) {
        if (session) {
          [session dealloc];
          session = nullptr;
        }

        if (error) {
          logger.error() << "Authentication failed:"
                         << QString::fromNSString([error localizedDescription]);

          if ([error code] == ASWebAuthenticationSessionErrorCodeCanceledLogin) {
            emit abortedByUser();
          } else {
            emit failed(ErrorHandler::AuthenticationError);
          }

          return;
        }

        if (!callbackURL) {
          logger.error() << "Authentication failed: missing callback URL";
          emit failed(ErrorHandler::AuthenticationError);
          return;
        }

        QUrl callbackUrl = QUrl::fromNSURL(callbackURL);
        logger.debug() << "Authentication completed:" << callbackUrl.toString();

        QUrlQuery callbackQuery(callbackUrl.query());
        if (!callbackQuery.hasQueryItem("code")) {
          logger.error() << "Authentication failed: missing code";
          emit failed(ErrorHandler::AuthenticationError);
          return;
        }

        emit completed(callbackQuery.queryItemValue("code"));
      }];

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
  UIView* view = resolvePresentationView();
  if (view) {
    session.presentationContextProvider = [[ContextProvider alloc] initWithUIView:view];
  } else {
    logger.error() << "No presentation anchor found for authentication session.";
    return;
  }
#endif

  if (![session start]) {
    [session dealloc];
    session = nullptr;

    logger.error() << "Authentication failed: session doesn't start.";
    emit failed(ErrorHandler::AuthenticationError);
  }
}
