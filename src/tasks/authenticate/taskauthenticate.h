#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"

#include <QNetworkReply>

class TaskAuthenticate final : public Task
{
public:
    explicit TaskAuthenticate() : Task("TaskAuthenticate") {}

    void run(MozillaVPN *vpn) override;

    void authenticationCompleted(MozillaVPN *vpn, const QByteArray &data);

private Q_SLOTS:
    void requestFailed(QNetworkReply::NetworkError);
    void requestCompleted(const QByteArray &);
};

#endif // TASKAUTHENTICATE_H
