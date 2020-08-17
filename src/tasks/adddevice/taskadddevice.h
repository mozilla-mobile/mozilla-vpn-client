#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include "task.h"

class TaskAddDevice final : public Task
{
public:
    TaskAddDevice();

    void run(MozillaVPN *vpn) override;
};

#endif // TASKADDDEVICE_H
