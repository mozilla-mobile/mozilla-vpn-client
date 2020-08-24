#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

class Controller : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateOff,
        StateConnecting,
        StateOn,
    };

    Q_ENUM(State)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    Controller();

    State state() const { return m_state; }

    Q_INVOKABLE void activate();

    Q_INVOKABLE void deactivate();

signals:
    void stateChanged();

private:
    State m_state;
};

#endif // CONTROLLER_H
