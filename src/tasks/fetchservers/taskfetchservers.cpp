#include "taskfetchservers.h"
#include "mozillavpn.h"
#include "servercountrymodel.h"
#include "serversfetcher.h"

#include <QDebug>

void TaskFetchServers::run(MozillaVPN *vpn)
{
    qDebug() << "Fetching servers";

    m_fetcher = new ServersFetcher(this);
    connect(m_fetcher, &ServersFetcher::completed, [this, vpn](const QByteArray &serverData) {
        vpn->serversFetched(serverData);
        emit completed();
    });

    m_fetcher->run(vpn);
}
