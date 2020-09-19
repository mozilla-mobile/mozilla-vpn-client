#include "taskauthenticate.h"
#include "errorhandler.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "tasks/authenticate/authenticationlistener.h"
#include "user.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

namespace {

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
    qDebug() << "TaskAuthenticate::Run";

    Q_ASSERT(!m_authenticationListener);

    QByteArray pkceCodeVerifier = generatePkceCodeVerifier();
    QByteArray pkceCodeChallenge
        = QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256).toBase64();
    Q_ASSERT(pkceCodeChallenge.length() == 44);

    qDebug() << "pkceCodeVerifier:" << pkceCodeVerifier;
    qDebug() << "pkceCodeChallenge:" << pkceCodeChallenge;

    m_authenticationListener = new AuthenticationListener(this);

    if (!m_authenticationListener->initialize()) {
        vpn->errorHandle(ErrorHandler::AuthenticationError);
        emit completed();
        return;
    }

    connect(m_authenticationListener,
            &AuthenticationListener::completed,
            [this, vpn, pkceCodeVerifier](const QString &pkceCodeSucces) {
                qDebug() << "Authentication completed with code:" << pkceCodeSucces;

                NetworkRequest *request
                    = NetworkRequest::createForAuthenticationVerification(vpn,
                                                                          pkceCodeSucces,
                                                                          pkceCodeVerifier);

                connect(request,
                        &NetworkRequest::requestFailed,
                        [this, vpn](QNetworkReply::NetworkError error) {
                            qDebug() << "Failed to complete the authentication" << this << error;
                            vpn->errorHandle(ErrorHandler::toErrorType(error));
                            emit completed();
                        });

                connect(request,
                        &NetworkRequest::requestCompleted,
                        [this, vpn](const QByteArray &data) {
                            qDebug() << "Authentication completed";
                            authenticationCompleted(vpn, data);
                        });
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

#ifdef IOS_INTEGRATION
    query.addQueryItem("platform", "ios");
#endif

    m_authenticationListener->setQueryItems(query);
    url.setQuery(query);

    QDesktopServices::openUrl(url.toString());
}

void TaskAuthenticate::authenticationCompleted(MozillaVPN *vpn, const QByteArray &data)
{
    qDebug() << "Authentication completed with data:" << data;

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
