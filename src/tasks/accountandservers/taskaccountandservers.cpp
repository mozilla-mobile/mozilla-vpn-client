#include "taskaccountandservers.h"
#include "errorhandler.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "servercountrymodel.h"
#include "serversfetcher.h"

#include <QDebug>

void TaskAccountAndServers::run(MozillaVPN *vpn)
{
    // Account fetch and servers fetch run in parallel.

    NetworkRequest *request = NetworkRequest::createForAccount(vpn);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Account request failed" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        m_accountCompleted = true;
        maybeCompleted();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &data) {
        qDebug() << "Account request completed";
        vpn->accountChecked(data);
        m_accountCompleted = true;
        maybeCompleted();
    });

    m_fetcher = new ServersFetcher(this);

    connect(m_fetcher, &ServersFetcher::failed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to fetch servers" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        m_serversCompleted = true;
        maybeCompleted();
    });

    connect(m_fetcher, &ServersFetcher::completed, [this, vpn](const QByteArray &serverData) {
        vpn->serversFetched(serverData);
        m_serversCompleted = true;
        maybeCompleted();
    });

    m_fetcher->run(vpn);
}

void TaskAccountAndServers::maybeCompleted()
{
    if (m_accountCompleted && m_serversCompleted) {
        emit completed();
    }
}
