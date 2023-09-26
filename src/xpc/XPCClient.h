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
#include <iostream>

#include "xpcbase.h"

class XPCClient : public QThread, private XPCBase {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCClient)

 public:
  XPCClient() : QThread() {}
  ~XPCClient(){};

  void run() override;

  void send(const QString msg);

  Q_SIGNAL void messageReceived(const QString msg);

 private:
  xpc_connection_t m_serverConnection;

  void handleServerEvent(xpc_object_t event);
};

#endif  // XPC_CLIENT_H
