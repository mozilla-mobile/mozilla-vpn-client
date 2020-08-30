#ifndef TIMERCONTROLLER_H
#define TIMERCONTROLLER_H

#include "controllerimpl.h"

#include <QTimer>

class TimerController : public ControllerImpl
{
    Q_OBJECT

public:
    TimerController(ControllerImpl *impl);

    void activate(const Server &server,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(const Server &server,
                    const Device *device,
                    const Keys *keys,
                    bool forSwitching) override;

private Q_SLOTS:
    void timeout();
    void maybeDone();

private:
    ControllerImpl *m_impl;
    QTimer m_timer;

    enum State {
        None,
        Connecting,
        Connected,
        Disconnecting,
        Disconnected,
    };

    State m_state = None;
};

#endif // TIMERCONTROLLER_H
