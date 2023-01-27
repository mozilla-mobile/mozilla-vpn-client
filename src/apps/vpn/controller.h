/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QTimer>
#include <functional>

#include "ipaddress.h"
#include "models/server.h"
#include "models/serverdata.h"
#include "pinghelper.h"

class ControllerImpl;
class MozillaVPN;

class HopConnection {
 public:
  HopConnection() {}

  Server m_server;
  int m_hopindex = 0;
  QList<IPAddress> m_allowedIPAddressRanges;
  QStringList m_excludedAddresses;
  QStringList m_vpnDisabledApps;
  QHostAddress m_dnsServer;
};

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
    StateSilentSwitching,
    StateSwitching,
  };
  Q_ENUM(State)

  enum Reason {
    ReasonNone = 0,
    ReasonSwitching,
    ReasonConfirming,
  };

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(qint64 time READ time NOTIFY timeChanged)
  Q_PROPERTY(
      int connectionRetry READ connectionRetry NOTIFY connectionRetryChanged);
  Q_PROPERTY(bool enableDisconnectInConfirming READ enableDisconnectInConfirming
                 NOTIFY enableDisconnectInConfirmingChanged);

#ifdef MZ_DUMMY
  // This is just for testing purposes. Not exposed in prod.
  Q_PROPERTY(QString currentServerString READ currentServerString NOTIFY
                 currentServerChanged);
#endif

 public:
  Controller();
  ~Controller();

  void initialize();

  State state() const;

  Q_INVOKABLE void logout();

  qint64 time() const;

  bool switchServers(const ServerData& serverData);
  bool silentSwitchServers(bool serverCoolDownNeeded);

  void updateRequired();

  void getBackendLogs(std::function<void(const QString& logs)>&& callback);

  void cleanupBackendLogs();

  void getStatus(
      std::function<void(const QString& serverIpv4Gateway,
                         const QString& deviceIpv4Address, uint64_t txBytes,
                         uint64_t rxBytes)>&& callback);

  int connectionRetry() const { return m_connectionRetry; }

  bool enableDisconnectInConfirming() const {
    return m_enableDisconnectInConfirming;
  }

  void backendFailure();
  void serverUnavailable();

  void captivePortalPresent();
  void captivePortalGone();

  const ServerData& currentServer() const { return m_serverData; }

#ifdef MZ_DUMMY
  QString currentServerString() const;
#endif

 public slots:
  // These 2 methods activate/deactivate the VPN. Return true if a signal will
  // be emitted at the end of the operation.
  bool activate(const ServerData& serverData);
  bool deactivate();

  Q_INVOKABLE void quit();

 private slots:
  void connected(const QString& pubkey,
                 const QDateTime& connectionTimestamp = QDateTime());
  void disconnected();
  void timerTimeout();
  void implInitialized(bool status, bool connected,
                       const QDateTime& connectionDate);
  void statusUpdated(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes);
  void handshakeTimeout();

 signals:
  void stateChanged();
  void timeChanged();
  void readyToQuit();
  void readyToUpdate();
  void readyToBackendFailure();
  void readyToServerUnavailable(bool pingReceived);
  void connectionRetryChanged();
  void enableDisconnectInConfirmingChanged();
  void activationBlockedForCaptivePortal();
  void handshakeFailed(const QString& serverHostname);

#ifdef MZ_DUMMY
  void currentServerChanged();
#endif

 private:
  void setState(State state);

  void maybeEnableDisconnectInConfirming();

  bool processNextStep();
  QList<IPAddress> getAllowedIPAddressRanges(const Server& server);
  QStringList getExcludedAddresses();

  void activateInternal(bool forceDNSPort = false);
  void activateNext();

  void clearRetryCounter();
  void clearConnectedTime();
  void resetConnectedTime();

  void serverDataChanged();

 private:
  State m_state = StateInitializing;

  QTimer m_timer;

  bool m_portalDetected = false;

  QDateTime m_connectedTimeInUTC;

  PingHelper m_ping_canary;
  bool m_ping_received = false;

  QScopedPointer<ControllerImpl> m_impl;

  QTimer m_connectingTimer;
  QTimer m_handshakeTimer;
  bool m_enableDisconnectInConfirming = false;

  enum NextStep {
    None,
    Quit,
    Update,
    Disconnect,
    BackendFailure,
    ServerUnavailable,
  };

  NextStep m_nextStep = None;

  // Server data can change while the controller is busy completing an
  // activation or a server switch because they are managed by the
  // SettingsHolder object and exposed to user interaction, addons, and JS.
  //
  // But the controller needs to know the location to use for the entire
  // duration of its tasks. When the client schedules a VPN activation,
  // `m_serverData` is set as a copy of the current `MozillaVPN::serverData()`,
  // ignoring further updates until the pending operations terminate. Instead,
  // `m_nextServerData` is set when a server-switch request is scheduled while
  // an activation operation is still in progress.
  //
  // At initialization time, these two member variables are set to
  // MozillaVPN::serverData() to do not let not initialize.
  //
  // Please, do not use MozillaVPN::serverData() in the controller!
  ServerData m_serverData;
  ServerData m_nextServerData;

  int m_connectionRetry = 0;

  QList<HopConnection> m_activationQueue;

  QList<std::function<void(const QString& serverIpv4Gateway,
                           const QString& deviceIpv4Address, uint64_t txBytes,
                           uint64_t rxBytes)>>
      m_getStatusCallbacks;

  bool m_connectedBeforeTransaction = false;
};

#endif  // CONTROLLER_H
