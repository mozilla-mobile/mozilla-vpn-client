/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "models/server.h"
#include "connectioncheck.h"

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>

#include <functional>

class ControllerImpl;
class MozillaVPN;
class IPAddressRange;

class Controller final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Controller)

 public:
  enum State {
    StateInitializing,
    StateOff,
    StateConnecting,
    StateConfirming,
    StateOn,
    StateDisconnecting,
    StateSwitching,
    StateSilentSwitching,
    StateConfirmingSilentSwitch,
  };
  Q_ENUM(State)

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(int time READ time NOTIFY timeChanged)
  Q_PROPERTY(QString currentCity READ currentCity NOTIFY stateChanged)
  Q_PROPERTY(QString switchingCity READ switchingCity NOTIFY stateChanged)
  Q_PROPERTY(
      int connectionRetry READ connectionRetry NOTIFY connectionRetryChanged);

 public:
  Controller();
  ~Controller();

  void initialize();

  State state() const;

  Q_INVOKABLE void changeServer(const QString& countryCode,
                                const QString& city);

  Q_INVOKABLE void logout();

  int time() const;

  const QString& currentCity() const { return m_currentCity; }

  const QString& switchingCountryCode() const { return m_switchingCountryCode; }

  const QString& switchingCity() const { return m_switchingCity; }

  void silentSwitchServers();

  void updateRequired();

  void getBackendLogs(std::function<void(const QString& logs)>&& callback);

  void cleanupBackendLogs();

  void getStatus(
      std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                         uint64_t rxBytes)>&& callback);

  int connectionRetry() const { return m_connectionRetry; }

  void backendFailure();

 public slots:
  // These 2 methods activate/deactivate the VPN. Return true if a signal will
  // be emitted at the end of the operation.
  bool activate();
  bool deactivate();

  Q_INVOKABLE void quit();

 private slots:
  void connected();
  void disconnected();
  void timerTimeout();
  void implInitialized(bool status, bool connected,
                       const QDateTime& connectionDate);
  void statusUpdated(const QString& serverIpv4Gateway, uint64_t txBytes,
                     uint64_t rxBytes);

  void connectionConfirmed();
  void connectionFailed();

 signals:
  void stateChanged();
  void timeChanged();
  void readyToQuit();
  void readyToUpdate();
  void readyToBackendFailure();
  void connectionRetryChanged();

 private:
  void setState(State state);

  bool processNextStep();

  QList<IPAddressRange> getAllowedIPAddressRanges(const Server& server);

  void activateInternal();

  void resetConnectionCheck();

  void heartbeatCompleted();

  void resetConnectionTimer();

 private:
  State m_state = StateInitializing;

  QTimer m_timer;

  QElapsedTimer m_connectionTimer;

  // The connection could have been started before the execution of the app.
  // This contains the number of seconds to add to m_connectionTimer to know the
  // real starting time. We don't use QDateTime (or anything similar) because we
  // need a monotonic timer.
  uint32_t m_connectionTimerExtraSecs = 0;

  QScopedPointer<ControllerImpl> m_impl;

  QString m_currentCity;

  QString m_switchingCountryCode;
  QString m_switchingCity;

  enum NextStep {
    None,
    Quit,
    Update,
    Disconnect,
    BackendFailure,
  };

  NextStep m_nextStep = None;

  ConnectionCheck m_connectionCheck;
  int m_connectionRetry = 0;

  enum ReconnectionStep {
    NoReconnection,
    ExpectDisconnection,
    ExpectHeartbeat,
  };

  ReconnectionStep m_reconnectionStep = NoReconnection;

  QList<std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                           uint64_t rxBytes)>>
      m_getStatusCallbacks;
};

#endif  // CONTROLLER_H
