/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPC_CLIENT_H
#define XPC_CLIENT_H

#include <stdio.h>
#include <xpc/xpc.h>

#include <QList>
#include <QObject>
#include <QScopeGuard>
#include <QThread>
#include <QtLogging>
#include <iostream>

#include "xpcbase.h"

/**
 * @brief Client to connect to XPC Services
 *
 * to establish a connection call {connectService}
 *
 * The client will spin up it's own mach event loop.
 * Therfore it's advised to move this object into
 * it's own worker thread.
 */
class XPCClient : public QObject, private XPCBase {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCClient)

 public:
  XPCClient();
  ~XPCClient(){};

  /**
   * @brief Send a String to the connected service.
   * This function may be called from any thread.
   *
   * @param message - the message to be send.
   */
  Q_INVOKABLE void send(const QString message);

  /**
   * @brief Prompt's to connect to an XPC service
   * This function may be callsed from any thread.
   *
   * @param service - the full id of the service
   */
  Q_INVOKABLE void connectService(QString service);

 signals:
  /**
   * @brief Whenever a new message is recieved
   * IMPORTANT: This event is dispatched by a Mach I/O thread
   * (bypassing the QThread affinity of the obj)
   * use Qt::QueuedConnection if you have expectations about the
   * rx-thread.
   *
   * @param message - the recieved sting.
   */
  void messageReceived(const QString message);

  void onConnectionError(const QString errorMsg);

 private:
  void connectServiceInternal(const QString service);

  xpc_connection_t m_serverConnection;

  void handleServerEvent(xpc_object_t event);

  void sendInternal(const QString message);

  bool isXPCThread() { return QThread::currentThread() == this->thread(); }

  Qt::HANDLE m_threadID = 0;
};

#endif  // XPC_CLIENT_H
