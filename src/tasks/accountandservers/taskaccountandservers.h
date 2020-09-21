#ifndef TASKACCOUNTANDSERVERS_H
#define TASKACCOUNTANDSERVERS_H

#include "task.h"

#include <QPointer>

class ServersFetcher;

class TaskAccountAndServers final : public Task
{
public:
    TaskAccountAndServers() : Task("TaskAccountAndServers") {}

    void run(MozillaVPN *vpn) override;

private:
    void maybeCompleted();

private:
    QPointer<ServersFetcher> m_fetcher;

    bool m_accountCompleted = false;
    bool m_serversCompleted = false;
};

#endif // TASKACCOUNTANDSERVERS_H
