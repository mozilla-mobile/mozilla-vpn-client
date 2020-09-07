#ifndef MACOSPINGSENDWORKER_H
#define MACOSPINGSENDWORKER_H

#include "pingsendworker.h"

class QSocketNotifier;

class MacosPingSendWorker final : public PingSendWorker
{
    Q_OBJECT

public Q_SLOTS:
    void sendPing(const QString &destination) override;
    void stopPing() override;

private:
    void releaseObjects();

private:
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socket = 0;
};

#endif // MACOSPINGSENDWORKER_H
