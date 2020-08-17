#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include "task.h"

class TaskAddDevice final : public Task
{
public:
    TaskAddDevice(const QString &deviceName) : Task("TaskAddDevice"), m_deviceName(deviceName) {}

    void run(MozillaVPN *vpn) override;

private:
    const QString m_deviceName;
};

#endif // TASKADDDEVICE_H
