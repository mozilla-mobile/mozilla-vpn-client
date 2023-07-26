/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

// #include "constants.h"
#include <QObject>

class MozillaVPN;

class ConnectionManager final : public QObject  {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ConnectionManager)
 
public:
  
//  enum ConnectionState {
//    ConnectionStateInternetProbe,
//    ConnectionStateServerProbe,
//    ConnectionStateFirewall,
//    ConnectionStateCaptivePortal,
//    ConnectionStateCheckSubscription,
//    ConnectionStateUnstable,
//    ConnectionStateIdle,       // Used when there are no active probes ongoing
//  };
//  Q_ENUM(ConnectionState)
  
  // These are temporarily copied from the controller state to mirror them, but will eventually be removed by ConnectionState above 
  enum ControllerState {
    StateInitializing,
    StateOff,
    StateCheckSubscription,
    StateConnecting,
    StateConfirming,
    StateOn,
    StateDisconnecting,
    StateSilentSwitching,
    StateSwitching,
  };
  Q_ENUM(ControllerState)
  
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
 Q_PROPERTY(bool silentServerSwitchingSupported READ
                silentServerSwitchingSupported CONSTANT);

#ifdef MZ_DUMMY
 // This is just for testing purposes. Not exposed in prod.
 Q_PROPERTY(QString currentServerString READ currentServerString NOTIFY
                currentServerChanged);
#endif
  
public:
  ConnectionManager();
 ~ConnectionManager();

 void initialize();


};  // namespace ConnectionManager

#endif  // CONNECTIONMANAGER_H
