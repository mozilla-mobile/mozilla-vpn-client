/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H

#include <QObject>
#include <functional>

#include "controller.h"

class Keys;
class Device;
class Server;
class QDateTime;
class IPAddress;
class QHostAddress;
class QJsonObject;

// This object is allocated when the VPN is about to be activated.
// It's kept alive, basically forever, except in these scenarios, in which it's
// recreated:
// - the user does a logout
// - there is an authentication falure
class ControllerImpl : public QObject {
  Q_OBJECT

 public:
  ControllerImpl() = default;

  virtual ~ControllerImpl() = default;

  // This method is called to initialize the controller. The initialization
  // is completed when the signal "initialized" is emitted.
  virtual void initialize(const Device* device, const Keys* keys) = 0;

  // This method is called when the VPN client needs to activate the VPN
  // tunnel. It's called only at the end of the initialization process.  When
  // this method is called, the VPN client is in "connecting" state.  This
  // state terminates when the "connected" (or the "disconnected") signal is
  // received.
  virtual void activate(const InterfaceConfig& config,
                        Controller::Reason Reason) = 0;

  // This method terminates the VPN tunnel. The VPN client is in
  // "disconnecting" state until the "disconnected" signal is received.
  virtual void deactivate() = 0;

  // This method is used to remove the tunnel config from the operating
  // system. This is used upon logging out and resetting, so the
  // tunnel cannot be reactivated in system settings.
  virtual void deleteOSTunnelConfig(){};

  // This method attempts to force the daemon to crash, and is used for
  // testing the ability of the VPN to recover from a backend error.
  virtual void forceDaemonCrash() {}

  // This method attempts to force the daemon to do a silent server switch,
  // and is used exclusively for testing.
  virtual void forceDaemonSilentServerSwitch(){};

  // This method is used to retrieve the VPN tunnel status (mainly the number
  // of bytes sent and received). It's called always when the VPN tunnel is
  // active.
  virtual void checkStatus() = 0;

  // This method is used to retrieve the logs from the backend service. The logs
  // will be written to the QIODevice, and closed at the end of the logs.
  virtual void getBackendLogs(QIODevice* device);

  // Cleanup the backend logs.
  virtual void cleanupBackendLogs(){};

  // Whether the controller supports multihop
  virtual bool multihopSupported() { return false; }

  virtual bool silentServerSwitchingSupported() const { return true; }

 protected:
  // Helper method - process a JSON status and emit the statusUpdated signal.
  void emitStatusFromJson(const QJsonObject& obj);

 signals:
  // This signal is emitted when the controller is initialized. Note that the
  // VPN tunnel can be already active. In this case, "connected" should be set
  // to true and the "connectionDate" should be set to the activation date if
  // known.
  // If "status" is set to false, the backend service is considered unavailable.
  void initialized(bool status, bool connected,
                   const QDateTime& connectionDate);

  // This signal is emitted when the controller is unable to initialize due to
  // a missing permission. The user may need to perform an action in their
  // system settings to enable the VPN backend.
  void permissionRequired();

  // These 2 signals can be dispatched at any time.
  void connected(const QString& pubkey,
                 const QDateTime& connectionTimestamp = QDateTime());
  void disconnected();

  // This method should be emitted after a checkStatus() call.
  // "serverIpv4Gateway" is the current VPN tunnel gateway.
  // "deviceIpv4Address" is the address of the VPN client.
  // "txBytes" and "rxBytes" contain the number of transmitted and received
  // bytes since the last statusUpdated signal.
  void statusUpdated(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes);

  // This signal is emitted when the implementation encounters an error.
  void backendFailure(Controller::ErrorCode errorCode);
};

#endif  // CONTROLLERIMPL_H
