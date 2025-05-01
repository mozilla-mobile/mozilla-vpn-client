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
  QThread* m_thread = nullptr;
  void* m_listener = nullptr;
};

#endif  // XPCDAEMONSERVER_H
