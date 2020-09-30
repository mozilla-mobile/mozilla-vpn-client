#include "authenticationlistener.h"
#include "mozillavpn.h"

#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui>
#include <QWindow>
#include <QQmlApplicationEngine>

#import <UIKit/UIKit.h>
#import <AuthenticationServices/ASWebAuthenticationSession.h>

static ASWebAuthenticationSession *session = nullptr;

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

void AuthenticationListener::start(MozillaVPN* vpn, QUrl &url, QUrlQuery &query)
{
    qDebug() << "AuthenticationListener initialize" << session;

    query.addQueryItem("platform", "ios");

    // If we want to force the IAP settings:
    //query.addQueryItem("iap", "true");

    url.setQuery(query);

    qDebug() << url;
    Q_ASSERT(!session);
    session = [[ASWebAuthenticationSession alloc] initWithURL:url.toNSURL() callbackURLScheme:@"mozilla-vpn" completionHandler:^(NSURL * _Nullable callbackURL, NSError * _Nullable error) {
        [session dealloc];
        session = nullptr;

        if (error) {
            qDebug() << "Authentication failed:" << QString::fromNSString([error localizedDescription]);
            qDebug() << "Suggestion:" << QString::fromNSString([error localizedRecoverySuggestion]);
            qDebug() << "Reason:" << QString::fromNSString([error localizedFailureReason]);
            emit failed(ErrorHandler::AuthenticationError);
            return;
        }

        QUrl callbackUrl = QUrl::fromNSURL(callbackURL);
        qDebug() << "Authentication completed:" << callbackUrl;

        Q_ASSERT(callbackUrl.hasQuery());

        QUrlQuery callbackUrlQuery(callbackUrl.query());
        Q_ASSERT(callbackUrlQuery.hasQueryItem("code"));
        QString code = callbackUrlQuery.queryItemValue("code");
        emit completed(code);
    }];

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    QObject *rootObject = vpn->engine()->rootObjects().first();
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

        qDebug() << "Authentication failed: session doesn't start.";
        emit failed(ErrorHandler::BackendServiceError);
    }
}
