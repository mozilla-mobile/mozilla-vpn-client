#include "taskaccount.h"
#include "errorhandler.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>

void TaskAccount::run(MozillaVPN *vpn)
{
    NetworkRequest *request = NetworkRequest::createForAccount(vpn);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Account request failed" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &data) {
        qDebug() << "Account request completed";
        vpn->accountChecked(data);
        emit completed();
    });
}
