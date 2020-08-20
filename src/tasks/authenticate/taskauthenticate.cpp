#include "taskauthenticate.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "taskauthenticationverifier.h"
#include "user.h"

#include <QDebug>
#include <QDesktopServices>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>

void TaskAuthenticate::run(MozillaVPN *vpn)
{
    Q_ASSERT(vpn);

    qDebug() << "TaskAuthenticate::Run";

    NetworkRequest *request = NetworkRequest::createForAuthenticate(vpn);
    qDebug() << request;

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Authentication failed: " << this << error;
        // TODO
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &data) {
        qDebug() << "Authentication request completed: " << this << data;

        QJsonDocument json = QJsonDocument::fromJson(data);
        if (json.isNull()) {
            qDebug() << "Invalid JSON object";
            // TODO
            return;
        }

        Q_ASSERT(json.isObject());
        QJsonObject obj = json.object();

        Q_ASSERT(obj.contains("login_url"));
        QJsonValue loginUrl = obj.take("login_url");
        Q_ASSERT(loginUrl.isString());

        Q_ASSERT(obj.contains("verification_url"));
        QJsonValue verificationUrl = obj.take("verification_url");
        Q_ASSERT(verificationUrl.isString());

        Q_ASSERT(obj.contains("poll_interval"));
        QJsonValue pollInterval = obj.take("poll_interval");
        Q_ASSERT(pollInterval.isDouble());

        qDebug() << "Opening the URL: " << loginUrl.toString();
        QDesktopServices::openUrl(loginUrl.toString());

        TaskAuthenticationVerifier *verifier
            = new TaskAuthenticationVerifier(this, verificationUrl.toString(), pollInterval.toInt());
        connect(verifier, &TaskAuthenticationVerifier::completed, [this, vpn](const QByteArray &data) {
            authenticationCompleted(vpn, data);
        });
    });
}

void TaskAuthenticate::authenticationCompleted(MozillaVPN *vpn, const QByteArray &data)
{
    qDebug() << "Authentication completed";

    QJsonDocument json = QJsonDocument::fromJson(data);
    if (json.isNull()) {
        qDebug() << "Invalid JSON object";
        // TODO
        return;
    }

    Q_ASSERT(json.isObject());
    QJsonObject obj = json.object();

    Q_ASSERT(obj.contains("user"));
    QJsonValue userObj = obj.take("user");
    Q_ASSERT(userObj.isObject());

    Q_ASSERT(obj.contains("token"));
    QJsonValue tokenValue = obj.take("token");
    Q_ASSERT(tokenValue.isString());

    QJsonObject userDataObj = userObj.toObject();
    vpn->authenticationCompleted(userDataObj, tokenValue.toString());

    emit completed();
}
