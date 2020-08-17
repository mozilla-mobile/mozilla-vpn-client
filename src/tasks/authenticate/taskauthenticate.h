#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"

#include <QNetworkReply>

class TaskAuthenticate final : public Task
{
public:
    explicit TaskAuthenticate() : Task("TaskAuthenticate") {}

    void run(MozillaVPN *vpn) override;

    void authenticationCompleted(MozillaVPN *vpn, QByteArray data);

private Q_SLOTS:
    void requestFailed(QNetworkReply::NetworkError);
    void requestCompleted(QByteArray);
};

#endif // TASKAUTHENTICATE_H
