#include "taskfetchservers.h"
#include "mozillavpn.h"
#include "serverdata.h"
#include "serversfetcher.h"

#include <QDebug>

void TaskFetchServers::run(MozillaVPN *vpn)
{
    qDebug() << "Fetching servers";

    m_fetcher = new ServersFetcher(this);
    connect(m_fetcher, &ServersFetcher::completed, [this, vpn](ServerData *data) {
        vpn->serversFetched(data);
        emit completed();
    });

    m_fetcher->run(vpn);
}
