#ifndef TASKFETCHSERVERS_H
#define TASKFETCHSERVERS_H

#include "task.h"

#include <QPointer>

class ServersFetcher;

class TaskFetchServers : public Task
{
public:
    TaskFetchServers() : Task("TaskFetchServers") {}

    void run(MozillaVPN *vpn) override;

private:
    QPointer<ServersFetcher> m_fetcher;
};

#endif // TASKFETCHSERVERS_H
