/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui>
#include <QWindow>
#include <QQmlApplicationEngine>

#import <UIKit/UIKit.h>
#import <AuthenticationServices/ASWebAuthenticationSession.h>

namespace {

Logger logger({LOG_IOS, LOG_MAIN}, "AuthenticationListener");

ASWebAuthenticationSession *session = nullptr;

} // namespace

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
@interface ContextProvider : NSObject <ASWebAuthenticationPresentationContextProviding> {
    UIView *m_view;
}
@end

@implementation ContextProvider

- (id) initWithUIView:(UIView *)uiView
{
    self = [super init];
    if (self) {
        m_view = uiView;
    }
    return self;
}

- (nonnull ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:(nonnull ASWebAuthenticationSession *)session {
    return m_view.window;
}

@end
#endif

// static
AuthenticationListener *AuthenticationListener::create(QObject *parent)
{
    return new AuthenticationListener(parent);
}

AuthenticationListener::AuthenticationListener(QObject *parent): QObject(parent) {}

void AuthenticationListener::start(MozillaVPN *vpn, QUrl &url, QUrlQuery &query)
{
    Q_UNUSED(vpn);

    logger.log() << "AuthenticationListener initialize";

    query.addQueryItem("platform", "ios");

    // If we want to force the IAP settings:
    //query.addQueryItem("iap", "true");

    url.setQuery(query);

    Q_ASSERT(!session);
    session = [[ASWebAuthenticationSession alloc] initWithURL:url.toNSURL() callbackURLScheme:@"mozilla-vpn" completionHandler:^(NSURL * _Nullable callbackURL, NSError * _Nullable error) {
        [session dealloc];
        session = nullptr;

        if (error) {
            logger.log() << "Authentication failed:"
                         << QString::fromNSString([error localizedDescription]);
            logger.log() << "Suggestion:"
                         << QString::fromNSString([error localizedRecoverySuggestion]);
            logger.log() << "Reason:" << QString::fromNSString([error localizedFailureReason]);

            if ([error code] == ASWebAuthenticationSessionErrorCodeCanceledLogin) {
                emit abortedByUser();
            } else {
                emit failed(ErrorHandler::BackendServiceError);
            }

            return;
        }

        QUrl callbackUrl = QUrl::fromNSURL(callbackURL);
        logger.log() << "Authentication completed:" << callbackUrl.toString();

        Q_ASSERT(callbackUrl.hasQuery());

        QUrlQuery callbackUrlQuery(callbackUrl.query());
        Q_ASSERT(callbackUrlQuery.hasQueryItem("code"));
        QString code = callbackUrlQuery.queryItemValue("code");
        emit completed(code);
    }];

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    QObject *rootObject = QmlEngineHolder::instance()->engine()->rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(rootObject);
    Q_ASSERT(window);

    UIView *view = static_cast<UIView *>(
                QGuiApplication::platformNativeInterface()
                ->nativeResourceForWindow("uiview", window));

    session.presentationContextProvider = [[ContextProvider alloc] initWithUIView: view];
 #endif

    if (![session start]) {
        [session dealloc];
        session = nullptr;

        logger.log() << "Authentication failed: session doesn't start.";
        emit failed(ErrorHandler::BackendServiceError);
    }
}
