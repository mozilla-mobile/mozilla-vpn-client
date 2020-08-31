#ifndef DUMMYPINGSENDWORKER_H
#define DUMMYPINGSENDWORKER_H

#include "pingsendworker.h"

class QTimer;

class DummyPingSendWorker final : public PingSendWorker
{
    Q_OBJECT

public:
    DummyPingSendWorker();

public Q_SLOTS:
    void sendPing(const QString &destination) override;
    void stopPing() override;

private:
    QTimer *m_timer = nullptr;

    enum State {
        Good,
        Unstable,
        NoSignal,
    };

    State m_state;
};

#endif // DUMMYPINGSENDWORKER_H
