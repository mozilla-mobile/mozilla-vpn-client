/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCDAEMONSERVER_H
#define XPCDAEMONSERVER_H

#include <QObject>

#include "daemon/daemon.h"

class XpcDaemonServer final : public QObject {
  Q_OBJECT

 public:
  XpcDaemonServer(Daemon* daemon);
  ~XpcDaemonServer();

 private:
  void* m_listener = nullptr;
};

// And a little helper to manage async responses.
class XpcSessionBridge final : public QObject {
  Q_OBJECT

 public:
  XpcSessionBridge(Daemon* daemon, void* connection);

 public slots:
  void connected(const QString& pubkey);
  void disconnected();

 private:
  void* m_connection = nullptr;
};

#endif  // XPCDAEMONSERVER_H
