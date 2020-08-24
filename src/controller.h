#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "controllerimpl.h"

#include <QObject>

class QTimer;
class MozillaVPN;

class Controller final : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateOff,
        StateConnecting,
        StateOn,
        StateDisconnecting,
    };

    Q_ENUM(State)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(int time READ time NOTIFY timeChanged)

public:
    Controller();

    void setVPN(MozillaVPN *vpn);

    State state() const { return m_state; }

    Q_INVOKABLE void activate();

    Q_INVOKABLE void deactivate();

    int time() const { return m_time; }

private Q_SLOTS:
    void connected();
    void disconnected();
    void timeUpdated();

signals:
    void stateChanged();
    void timeChanged();

private:
    State m_state = StateOff;

    MozillaVPN *m_vpn = nullptr;

    QTimer *m_timer = nullptr;
    int m_time = 0;

    QScopedPointer<ControllerImpl> m_impl;
};

#endif // CONTROLLER_H
