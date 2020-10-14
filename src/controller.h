/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "server.h"

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include <functional>

class ControllerImpl;
class MozillaVPN;

class Controller final : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateInitializing,
        StateOff,
        StateConnecting,
        StateOn,
        StateDisconnecting,
        StateSwitching,
        StateDeviceLimit,
    };
    Q_ENUM(State)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(int time READ time NOTIFY timeChanged)
    Q_PROPERTY(QString currentCity READ currentCity NOTIFY stateChanged)
    Q_PROPERTY(QString switchingCity READ switchingCity NOTIFY stateChanged)

public:
    Controller();
    ~Controller();

    void initialize();

    State state() const { return m_state; }

    Q_INVOKABLE void activate();

    Q_INVOKABLE void deactivate();

    Q_INVOKABLE void changeServer(const QString &countryCode, const QString &city);

    Q_INVOKABLE void quit();

    Q_INVOKABLE void logout();

    int time() const;

    const QString &currentCity() const { return m_currentCity; }

    const QString &switchingCity() const { return m_switchingCity; }

    void setDeviceLimit(bool deviceLimit);

    bool isDeviceLimit() const { return m_state == StateDeviceLimit; }

    void updateRequired();

    void subscriptionNeeded();

    void getBackendLogs(std::function<void(const QString &logs)> &&callback);

    void getStatus(
        std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
            &&callback);

public slots:
    void captivePortalDetected();

private slots:
    void connected();
    void disconnected();
    void timerTimeout();
    void implInitialized(bool status, State state, const QDateTime &connectionDate);
    void statusUpdated(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes);

signals:
    void initialized();
    void stateChanged();
    void timeChanged();
    void readyToQuit();
    void readyToUpdate();
    void readyToSubscribe();

private:
    void setState(State state);

    bool processNextStep();

private:
    State m_state = StateInitializing;

    QTimer m_timer;

    QDateTime m_connectionDate;

    QScopedPointer<ControllerImpl> m_impl;

    QString m_currentCity;

    QString m_switchingCountryCode;
    QString m_switchingCity;

    enum NextStep {
        None,
        Quit,
        Update,
        Disconnect,
        DeviceLimit,
        Subscribe,
        WaitForCaptivePortal,
    };

    NextStep m_nextStep = None;

    QList<std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>>
        m_getStatusCallbacks;
};

#endif // CONTROLLER_H
