#ifndef LINUXPINGSENDER_H
#define LINUXPINGSENDER_H

#include "pingsender.h"

#include <QThread>

class LinuxPingSender : public PingSender
{
    Q_OBJECT

public:
    LinuxPingSender(QObject *parent);

    ~LinuxPingSender();

    void send(const QString &destination) override;

    void stop() override;

signals:
    void sendPing(const QString &destination);
    void stopPing();

private Q_SLOTS:
    void pingFailed();
    void pingSucceeded();

private:
    QThread m_workerThread;

    bool m_active = false;
};

#endif // LINUXPINGSENDER_H
