#include "serversfetcher.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "serverdata.h"

void ServersFetcher::run(MozillaVPN *vpn)
{
    NetworkRequest *request = NetworkRequest::createForServers(vpn);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to retrieve servers" << this << error;
        // TODO
    });

    connect(request, &NetworkRequest::requestCompleted, [this](QByteArray data) {
        qDebug() << "Servers obtained";
        ServerData *servers = ServerData::fromJson(data);
        emit completed(servers);
    });
}
