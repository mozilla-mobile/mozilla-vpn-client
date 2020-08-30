#ifndef DUMMYPINGSENDER_H
#define DUMMYPINGSENDER_H

#include "pingsender.h"
#include <QTimer>

class DummyPingSender : public PingSender
{
public:
    DummyPingSender(QObject *parent);

    void send(const QString &destination) override;

    void stop() override;

private:
    QTimer m_timer;

    enum State {
        Good,
        Unstable,
        NoSignal,
    };

    State m_state;
};

#endif // DUMMYPINGSENDER_H
