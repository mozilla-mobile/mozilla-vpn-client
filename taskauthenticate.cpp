#include "taskauthenticate.h"
#include <QDebug>

void TaskAuthenticate::Run()
{
    qDebug() << "TaskAuthenticate::Run";

    // TODO
    emit completed();
}
