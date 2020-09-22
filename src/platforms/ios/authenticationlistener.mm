#include "authenticationlistener.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

#import <UIKit/UIKit.h>
#import <AuthenticationServices/ASWebAuthenticationSession.h>

static ASWebAuthenticationSession *session = nullptr;

void AuthenticationListener::start(QUrl &url, QUrlQuery &query)
{
    qDebug() << "AuthenticationListener initialize" << session;

    query.addQueryItem("platform", "ios");
    url.setQuery(query);

    Q_ASSERT(!session);
    session = [[ASWebAuthenticationSession alloc] initWithURL:url.toNSURL() callbackURLScheme:@"mozillavpn" completionHandler:^(NSURL * _Nullable callbackURL, NSError * _Nullable error) {
        [session dealloc];
        session = nullptr;

        if (error) {
            qDebug() << "Authentication failed:" << QString::fromNSString([error localizedDescription]);
            qDebug() << "Suggestion:" << QString::fromNSString([error localizedRecoverySuggestion]);
            qDebug() << "Reason:" << QString::fromNSString([error localizedFailureReason]);
            emit failed(ErrorHandler::BackendServiceError);
            return;
        }

        QUrl callbackUrl = QUrl::fromNSURL(callbackURL);
        qDebug() << "Authentication completed:" << callbackUrl;

        // TODO
    }];

    // TODO:
    //session.presentationContextProvider = this;

    if (![session start]) {
        [session dealloc];
        session = nullptr;

        qDebug() << "Authentication failed: session doesn't start.";
        emit failed(ErrorHandler::BackendServiceError);
    }
}
