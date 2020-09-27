#ifndef TIMERCONTROLLER_H
#define TIMERCONTROLLER_H

#include "controllerimpl.h"

#include <QTimer>

constexpr uint32_t TIME_ACTIVATION = 1000;
constexpr uint32_t TIME_DEACTIVATION = 1500;
constexpr uint32_t TIME_SWITCHING = 2000;

class TimerController : public ControllerImpl
{
    Q_OBJECT

public:
    TimerController(ControllerImpl *impl);

    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &server,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

private Q_SLOTS:
    void timeout();

private:
    void maybeDone(bool isConnected);

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
