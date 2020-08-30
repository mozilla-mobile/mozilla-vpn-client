#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include <QObject>
#include <QTimer>

class Server;
class PingSender;

class ConnectionHealth : public QObject
{
    Q_OBJECT

public:
    ConnectionHealth();

    void start(const Server &server);

    void stop();

private Q_SLOTS:
    void pingCompleted();

signals:
    void connectionStable();
    void connectionUnstable();
    void connectionNoSignal();

private:
    void sendPing();
    void wait();

private:
    enum State {
        // Ping sent, but not received yet.
        Pending,

        // The ping response has taken too much.
        Timeout,

        // We are waiting for the next ping to be sent.
        Waiting,

        // All disabled.
        Inactive,
    };

    State m_state = Inactive;

    QString m_gateway;

    QTimer m_unstableTimer;
    QTimer m_noSignalTimer;
    QTimer m_waitingTimer;

    PingSender *m_pingSender = nullptr;
};

#endif // CONNECTIONHEALTH_H
