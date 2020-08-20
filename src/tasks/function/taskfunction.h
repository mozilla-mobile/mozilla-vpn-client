#ifndef TASKFUNCTION_H
#define TASKFUNCTION_H

#include <functional>

#include "task.h"

class TaskFunction : public Task
{
public:
    TaskFunction(std::function<void(MozillaVPN *)> &&callback)
        : Task("TaskFunction"), m_callback(std::move(callback))
    {}

    void run(MozillaVPN *vpn) override
    {
        m_callback(vpn);
        emit completed();
    }

private:
    std::function<void(MozillaVPN *)> m_callback;
};

#endif // TASKFUNCTION_H
