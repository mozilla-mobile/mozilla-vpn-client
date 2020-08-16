#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"

class TaskAuthenticate final : public Task
{
public:
    explicit TaskAuthenticate() : Task("TaskAuthenticate") {}

    ~TaskAuthenticate() = default;

    void Run(MozillaVPN* aVPN) override;
};

#endif // TASKAUTHENTICATE_H
