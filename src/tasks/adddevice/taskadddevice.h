#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include "task.h"

class TaskAddDevice final : public Task
{
public:
    explicit TaskAddDevice(const QString &deviceName);

    void run(MozillaVPN *vpn) override;

private:
    QString m_deviceName;
};

#endif // TASKADDDEVICE_H
