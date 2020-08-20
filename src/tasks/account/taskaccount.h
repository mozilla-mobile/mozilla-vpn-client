#ifndef TASKACCOUNT_H
#define TASKACCOUNT_H

#include "task.h"

class TaskAccount : public Task
{
public:
    TaskAccount() : Task("TaskAccount") {}

    void run(MozillaVPN *vpn) override;
};

#endif // TASKACCOUNT_H
