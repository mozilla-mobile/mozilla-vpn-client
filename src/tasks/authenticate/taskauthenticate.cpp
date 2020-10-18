/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"
#include "errorhandler.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "user.h"

#ifdef IOS_INTEGRATION
#include "platforms/ios/authenticationlistener.h"
#else
#include "tasks/authenticate/authenticationlistener.h"
#endif

#include <QCryptographicHash>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

namespace {

Logger logger(LOG_MAIN, "TaskAuthenticate");

QByteArray generatePkceCodeVerifier()
{
    QRandomGenerator *generator = QRandomGenerator::system();
    Q_ASSERT(generator);

    QByteArray pkceCodeVerifier;
    static QByteArray range("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
    for (uint16_t i = 0; i < 128; ++i) {
        pkceCodeVerifier.append(range.at(generator->generate() % range.length()));
    }

    return pkceCodeVerifier;
}

} // anonymous namespace

void TaskAuthenticate::run(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);
    logger.log() << "TaskAuthenticate::Run";

    Q_ASSERT(!m_authenticationListener);

    QByteArray pkceCodeVerifier = generatePkceCodeVerifier();
    QByteArray pkceCodeChallenge
        = QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256).toBase64();
    Q_ASSERT(pkceCodeChallenge.length() == 44);

    logger.log() << "pkceCodeVerifier:" << pkceCodeVerifier;
    logger.log() << "pkceCodeChallenge:" << pkceCodeChallenge;

    m_authenticationListener = new AuthenticationListener(this);

    connect(m_authenticationListener,
            &AuthenticationListener::completed,
            [this, vpn, pkceCodeVerifier](const QString &pkceCodeSucces) {
                logger.log() << "Authentication completed with code:" << pkceCodeSucces;

                NetworkRequest *request
                    = NetworkRequest::createForAuthenticationVerification(vpn,
                                                                          pkceCodeSucces,
                                                                          pkceCodeVerifier);

                connect(request,
                        &NetworkRequest::requestFailed,
                        [this, vpn](QNetworkReply::NetworkError error) {
                            logger.log() << "Failed to complete the authentication" << error;
                            vpn->errorHandle(ErrorHandler::toErrorType(error));
                            emit completed();
                        });

                connect(request,
                        &NetworkRequest::requestCompleted,
                        [this, vpn](const QByteArray &data) {
                            logger.log() << "Authentication completed";
                            authenticationCompleted(vpn, data);
                        });
            });

    connect(m_authenticationListener,
            &AuthenticationListener::failed,
            [this, vpn](const ErrorHandler::ErrorType error) {
        vpn->errorHandle(error);
        emit completed();
    });

    QString path("/api/v2/vpn/login/");

#ifdef IOS_INTEGRATION
    path.append("ios");
#elif __linux__
    path.append("linux");
#elif __APPLE__
    path.append("macos");
#endif

    QUrl url(vpn->getApiUrl());
    url.setPath(path);

    QUrlQuery query;
    query.addQueryItem("code_challenge", QUrl::toPercentEncoding(pkceCodeChallenge));
    query.addQueryItem("code_challenge_method", "S256");

    m_authenticationListener->start(vpn, url, query);
}

void TaskAuthenticate::authenticationCompleted(MozillaVPN *vpn, const QByteArray &data)
{
    logger.log() << "Authentication completed with data:" << data;

    QJsonDocument json = QJsonDocument::fromJson(data);
    Q_ASSERT(!json.isNull());

    Q_ASSERT(json.isObject());
    QJsonObject obj = json.object();

    Q_ASSERT(obj.contains("user"));
    QJsonValue userObj = obj.take("user");
    Q_ASSERT(userObj.isObject());

    Q_ASSERT(obj.contains("token"));
    QJsonValue tokenValue = obj.take("token");
    Q_ASSERT(tokenValue.isString());

    QJsonDocument userDoc;
    userDoc.setObject(userObj.toObject());

    vpn->authenticationCompleted(userDoc.toJson(), tokenValue.toString());

    emit completed();
}
