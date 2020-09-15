#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"

class QByteArray;
class AuthenticationListener;

class TaskAuthenticate final : public Task
{
    Q_OBJECT

public:
    TaskAuthenticate() : Task("TaskAuthenticate") {}

    void run(MozillaVPN *vpn) override;

private:
    void authenticationCompleted(MozillaVPN *vpn, const QByteArray &data);

private:
    AuthenticationListener *m_authenticationListener = nullptr;
};

#endif // TASKAUTHENTICATE_H
