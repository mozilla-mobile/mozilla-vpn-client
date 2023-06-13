/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <QObject>

class MozillaVPN;

class ConnectivityManager final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectivityManager)

 public:
  enum ConnectionState {
    ConnectionStateOff,  // Corresponds to controller states: StateInitializing,
                         // StateOff
    ConnectionStateInternetProbe,      // Corresponds to controller states:
                                       // StateConnecting
    ConnectionStateServerProbe,        // Corresponds to controller states:
                                       // StateConnecting
    ConnectionStateCaptivePortal,      // Corresponds to controller states:
                                       // StateConnecting
    ConnectionStateCheckSubscription,  // Corresponds to controller states:
                                       // StateCheckSubscription
    ConnectionStateOn,  // Corresponds to controller state: StateOn

    // Mirrored from Controller states
    ConnectionStateSilentSwitching,  // Corresponds to StateSilentSwitching
    ConnectionStateSwitching,        // Corresponds to StateSwitching
    ConnectionStateConfirming,       // Corresponds to controller states:
                                     // StateConfirming
    ConnectionStateDisconnecting     // Corresponds to controller state:
                                     // StateDisconnecting
  };
  Q_ENUM(ConnectionState)

  enum ConnectionErrorState {
    ConnectionErrorStateInternetProbeFailed,
    ConnectionErrorStateServerProbeFailed,
    ConnectionErrorStateFirewallProbeFailed,
    ConnectionErrorStateCaptivePortalFailed
  };
  Q_ENUM(ConnectionErrorState)

 public:
  ConnectivityManager();
  ~ConnectivityManager();

  void initialize();

 private:
  ConnectionState m_connectionState = ConnectionStateOff;
};

#endif  // CONNECTIVITYMANAGER_H
