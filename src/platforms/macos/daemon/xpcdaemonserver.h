/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCDAEMONSERVER_H
#define XPCDAEMONSERVER_H

#include <objc/objc-runtime.h>

#include <QAtomicInt>
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
class XpcDaemonSession final : public QObject {
  Q_OBJECT

 public:
  XpcDaemonSession(Daemon* daemon, void* connection);

 public slots:
  void connected(const QString& pubkey);
  void disconnected();
  void backendFailure(DaemonError reason);

 private:
  void invokeClient(SEL selector);
  template<typename T> void invokeClient(SEL selector, T arg);

  QAtomicInt m_backlog;
  void* m_connection = nullptr;
};

#endif  // XPCDAEMONSERVER_H
