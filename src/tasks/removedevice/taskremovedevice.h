#ifndef TASKREMOVEDEVICE_H
#define TASKREMOVEDEVICE_H

#include "task.h"

class TaskRemoveDevice final : public Task
{
public:
    explicit TaskRemoveDevice(const QString &deviceName);

    void run(MozillaVPN *vpn) override;

private:
    QString m_deviceName;
};

#endif // TASKREMOVEDEVICE_H
