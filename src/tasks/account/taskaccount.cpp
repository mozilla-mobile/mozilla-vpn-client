#include "taskaccount.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

void TaskAccount::run(MozillaVPN *vpn)
{
    NetworkRequest *request = NetworkRequest::createForAccount(vpn);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to check the account status" << this << error;
        // TODO
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
