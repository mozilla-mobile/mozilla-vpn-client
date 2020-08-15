#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"
#include <QDebug>

class TaskAuthenticate final : public Task
{
public:
    explicit TaskAuthenticate() : Task("TaskAuthenticate") { qDebug() << "CTOR"; }

    ~TaskAuthenticate() { qDebug() << "DTOR"; }

    void Run() override;
};

#endif // TASKAUTHENTICATE_H
