/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QTimer>

#include "interfaceconfig.h"
#include "ipaddress.h"
#include "loghandler.h"
#include "models/server.h"
#include "models/serverdata.h"
#include "pinghelper.h"

class Controller;
class ControllerImpl;

class Controller : public QObject, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Controller)

 public:
  // Note - these states are ordered from least to most active.
  enum State {
    StateInitializing = 0,
    StatePermissionRequired,
    StateOff,
    StateDisconnecting,
    StateConnecting,
    StateConfirming,
    StateSwitching,
    StateSilentSwitching,
    StateOnPartial,
    StateOn,
  };
  Q_ENUM(State)

  enum Reason {
    ReasonNone = 0,
    ReasonSwitching,
    ReasonConfirming,
  };
  /**
   * @brief Who asked the Connection
   * to be Initiated? A Webextension
   * or a User Interaction inside the Client?
   */
  enum ActivationPrincipal {
    Null = 0,
    ExtensionUser = 1,
    ClientUser = 2,
  };
  Q_ENUM(ActivationPrincipal)

 public:
  qint64 time() const;
  qint64 connectionTimestamp() const;
  void serverUnavailable();
  void captivePortalPresent();
  void captivePortalGone();
  bool switchServers(const ServerData& serverData);
  void updateRequired();
  void deleteOSTunnelConfig();
  void startHandshakeTimer();
  bool isDeviceConnected() const { return m_isDeviceConnected; }
  bool isInitialized() const { return m_state >= StateOff; }
  bool isActive() const { return m_state > StateOff; }

  const ServerData& currentServer() const { return m_serverData; }

  bool enableDisconnectInConfirming() const {
    return m_enableDisconnectInConfirming;
  }

  enum ServerSelectionPolicy {
    RandomizeServerSelection,
    DoNotRandomizeServerSelection,
  };

  int connectionRetry() const { return m_connectionRetry; }
  State state() const;
  bool silentServerSwitchingSupported() const;
  void cleanupBackendLogs();

  // LogSerializer interface
  void serializeLogs(
      std::function<void(const QString& name, const QString& logs)>&& callback)
      override;

  void getStatus(
      std::function<void(const QString& serverIpv4Gateway,
                         const QString& deviceIpv4Address, uint64_t txBytes,
                         uint64_t rxBytes)>&& callback);

  QString currentServerString() const;

 public slots:
  // These 2 methods activate/deactivate the VPN. Return true if a signal will
  // be emitted at the end of the operation.
  bool activate(
      const ServerData& serverData, ActivationPrincipal = ClientUser,
      ServerSelectionPolicy serverSelectionPolicy = RandomizeServerSelection);
  bool deactivate(ActivationPrincipal = ClientUser);

  Q_INVOKABLE void quit();
  Q_INVOKABLE void forceDaemonCrash();
  Q_INVOKABLE void forceDaemonSilentServerSwitch();

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(qint64 time READ time NOTIFY timeChanged)
  Q_PROPERTY(
      int connectionRetry READ connectionRetry NOTIFY connectionRetryChanged);
  Q_PROPERTY(bool enableDisconnectInConfirming READ enableDisconnectInConfirming
                 NOTIFY enableDisconnectInConfirmingChanged);
  Q_PROPERTY(bool silentServerSwitchingSupported READ
                 silentServerSwitchingSupported CONSTANT);
  Q_PROPERTY(bool isDeviceConnected READ isDeviceConnected NOTIFY
                 isDeviceConnectedChanged);

  // This is just for testing purposes.
  Q_PROPERTY(QString currentServerString READ currentServerString NOTIFY
                 currentServerChanged);

 private slots:
  void timerTimeout();
  void handshakeTimeout();
  void connected(const QString& pubkey);
  void disconnected();
  void statusUpdated(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes);
  void implInitialized(bool status, bool connected,
                       const QDateTime& connectionDate);
  void implPermRequired();

 signals:
  void stateChanged();
  void timeChanged();
  void enableDisconnectInConfirmingChanged();
  void connectionRetryChanged();
  void recordConnectionStartTelemetry();
  void recordConnectionEndTelemetry();
  void recordDataTransferTelemetry();
  void readyToQuit();
  void readyToUpdate();
  void readyToServerUnavailable(bool pingReceived);
  void activationBlockedForCaptivePortal();
  void isDeviceConnectedChanged();

  void currentServerChanged();

 public:
  Controller();
  ~Controller();

  void initialize();

  struct IPAddressList {
    QList<IPAddress> v6;
    QList<IPAddress> v4;

    QList<IPAddress> flatten() {
      QList<IPAddress> list;
      list.append(v6);
      list.append(v4);
      return list;
    }
  };
  static IPAddressList getExcludedIPAddressRanges();
  static QList<IPAddress> getAllowedIPAddressRanges(const Server& server);

  enum ServerCoolDownPolicyForSilentSwitch {
    eServerCoolDownNeeded,
    eServerCoolDownNotNeeded,
  };

  bool silentSwitchServers(
      ServerCoolDownPolicyForSilentSwitch serverCoolDownPolicy);

 private:
  enum NextStep {
    None,
    Quit,
    Update,
    Disconnect,
  };

  NextStep m_nextStep = None;

  enum DNSPortPolicy {
    ForceDNSPort,
    DoNotForceDNSPort,
  };

  void activateInternal(DNSPortPolicy dnsPort,
                        ServerSelectionPolicy serverSelectionPolicy,
                        ActivationPrincipal);

  void startTimerIfInactive();
  void clearConnectedTime();
  void clearRetryCounter();
  void activateNext();
  void setState(State state);
  void resetConnectedTime();
  bool processNextStep();
  void maybeEnableDisconnectInConfirming();
  void serverDataChanged();
  QString useLocalSocketPath() const;

 private:
  QTimer m_timer;
  QTimer m_connectingTimer;
  QTimer m_handshakeTimer;

  QDateTime m_connectedTimeInUTC;

  State m_state = StateInitializing;
  ActivationPrincipal m_initiator = Null;
  bool m_enableDisconnectInConfirming = false;
  QList<InterfaceConfig> m_activationQueue;
  int m_connectionRetry = 0;

  QScopedPointer<ControllerImpl> m_impl;
  bool m_portalDetected = false;
  bool m_isDeviceConnected = true;

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
  bool isSwitchingServer;

  PingHelper m_pingCanary;
  bool m_pingReceived = false;

  ServerSelectionPolicy m_nextServerSelectionPolicy = RandomizeServerSelection;

  QList<std::function<void(const QString& serverIpv4Gateway,
                           const QString& deviceIpv4Address, uint64_t txBytes,
                           uint64_t rxBytes)>>
      m_getStatusCallbacks;

};  // namespace Controller

#endif  // CONTROLLER_H
