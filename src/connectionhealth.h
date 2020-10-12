/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONHEALTH_H
#define CONNECTIONHEALTH_H

#include <QObject>
#include <QTimer>

class Server;
class PingSender;

class ConnectionHealth : public QObject
{
public:
    enum ConnectionStability {
        Stable,
        Unstable,
        NoSignal,
    };
    Q_ENUM(ConnectionStability)

private:
    Q_OBJECT

    Q_PROPERTY(ConnectionStability stability READ stability() NOTIFY stabilityChanged)

public:
    ConnectionHealth();

    void start(const QString &serverIpv4Gateway);

    void stop();

    bool isRunning() const { return m_state != Inactive; }

    ConnectionStability stability() const { return m_stability; }

public Q_SLOTS:
    void connectionStateChanged();

private Q_SLOTS:
    void pingCompleted();

signals:
    void stabilityChanged();

private:
    void sendPing();
    void wait();
    void setStability(ConnectionStability stability);

private:
    ConnectionStability m_stability = Stable;

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
