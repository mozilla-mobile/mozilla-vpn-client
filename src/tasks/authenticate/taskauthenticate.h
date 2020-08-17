#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"

#include <QNetworkReply>

class TaskAuthenticate final : public Task
{
public:
    explicit TaskAuthenticate() : Task("TaskAuthenticate") {}

    ~TaskAuthenticate() = default;

    void Run(MozillaVPN *aVPN) override;

private Q_SLOTS:
    void requestFailed(QNetworkReply::NetworkError);
    void requestCompleted(QByteArray);

    void authenticationCompleted(QByteArray);
};

#endif // TASKAUTHENTICATE_H
