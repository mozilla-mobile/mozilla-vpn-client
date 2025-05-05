/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCDAEMONSERVER_H
#define XPCDAEMONSERVER_H

#include <QObject>
#include <QAtomicInt>

#include <objc/objc-runtime.h>

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

  void invokeClient(SEL selector, const QString& arg = QString());

 public slots:
  void connected(const QString& pubkey);
  void disconnected();

 private:
  QAtomicInt m_backlog;
  void* m_connection = nullptr;
};

#endif  // XPCDAEMONSERVER_H
