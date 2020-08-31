#ifndef LINUXPINGSENDWORKER_H
#define LINUXPINGSENDWORKER_H

#include "pingsendworker.h"

class QSocketNotifier;

class LinuxPingSendWorker final : public PingSendWorker
{
    Q_OBJECT

public Q_SLOTS:
    void sendPing(const QString &destination) override;
    void stopPing() override;

private:
    QSocketNotifier *m_socketNotifier;
};

#endif // LINUXPINGSENDWORKER_H
