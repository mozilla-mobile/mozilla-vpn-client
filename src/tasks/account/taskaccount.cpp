#include "taskaccount.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

void TaskAccount::run(MozillaVPN *vpn)
{
    NetworkRequest *request = NetworkRequest::createForAccount(vpn);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Account request failed" << error;
        vpn->errorHandle(error);
        emit completed();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &data) {
        qDebug() << "Account request completed" << data;

        QJsonDocument json = QJsonDocument::fromJson(data);
        Q_ASSERT(json.isObject());
        QJsonObject obj = json.object();

        vpn->accountChecked(obj);
        emit completed();
    });
}
