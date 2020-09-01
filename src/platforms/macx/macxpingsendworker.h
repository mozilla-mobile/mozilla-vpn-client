#ifndef MACXPINGSENDWORKER_H
#define MACXPINGSENDWORKER_H

#include "pingsendworker.h"

class QSocketNotifier;

class MacxPingSendWorker final : public PingSendWorker
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

#endif // MACXPINGSENDWORKER_H
