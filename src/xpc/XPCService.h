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

class XPCService : public QThread, private XPCBase {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCService)

 public:
  explicit XPCService();
  ~XPCService();

  void run() override;
  void acceptConnectionRequest(xpc_connection_t client);

  void handleClientEvent(xpc_object_t event, xpc_connection_t client);

  void send(const QString msg);

  void closeClientConnection(xpc_connection_t client);

 signals:
  void messageReceived(const QString msg);

 private:
  QList<xpc_connection_t> m_clients;
};

#endif  // XPCSERVICE_H
