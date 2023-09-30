/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCSERVICE_H
#define XPCSERVICE_H

#include <stdio.h>
#include <xpc/xpc.h>

#include <QList>
#include <QObject>
#include <QScopeGuard>
#include <QThread>
#include <iostream>

#include "xpcbase.h"

/**
 * @brief A Class implementing simple ipc using XPC 
 * 
 * Note: this class spin's up a new apple event loop, you probably want to 
 * put this into it's own thread. 
 * 
 */
class XPCService : public QObject, private XPCBase {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCService)

 public:
  /**
   * @brief Construct a new XPCService object
   *
   * @param aServiceName - (optional) Name of the XPC Service to use.
   * Note: this must be a defined name in Library/LaunchDaemons/
   * Defaults to org.mozilla.macos.FirefoxVPN.daemon
   * @param aSigningRequirement - (optional) The signing requirement String
   * If empty, no requirement will be enforced.
   * Defaults to ""
   * https://developer.apple.com/documentation/technotes/tn3127-inside-code-signing-requirements
   */
  XPCService(QString aServiceName = vpnDaemonName,
             QString aSigningRequirement = "");
  ~XPCService();

  // Start listening for connections
  void start();
  // Stop all connections and stop listening for new ones. 
  void stop();

  void send(const QString msg);

 signals:
  /**
   * @brief Whenever a new message is received
   * IMPORTANT: This event is dispatched by a Mach I/O thread
   * (bypassing the QThread affinity of the obj)
   * use Qt::QueuedConnection if you have expectations about the
   * rx-thread.
   *
   * @param message - the received sting.
   */
  void messageReceived(const QString msg);

 private:
  void closeClientConnection(xpc_connection_t client);
  void handleClientEvent(xpc_object_t event, xpc_connection_t client);

  void acceptConnectionRequest(xpc_connection_t client);

  void maybeEnforceSigningRequirement(xpc_connection_t listener);

  xpc_connection_t m_listener;
  QList<xpc_connection_t> m_clients;
  QString mSigningRequirement;
  QString mServiceName;
};

#endif  // XPCSERVICE_H
