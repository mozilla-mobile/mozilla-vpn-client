/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMONLOCALSERVER_H
#define DAEMONLOCALSERVER_H

#include <QLocalServer>

#include "daemonaccesscontrol.h"

class DaemonLocalServer final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DaemonLocalServer)

 public:
  explicit DaemonLocalServer(QObject* parent);
  ~DaemonLocalServer();

  static DaemonLocalServer* instance();

  bool initialize();
  DaemonAccessControl* accessControl() { return &m_accessControl; }

 private:
  QString daemonPath() const;

 private:
  QLocalServer m_server;
  DaemonAccessControl m_accessControl;
};

#endif  // DAEMONLOCALSERVER_H
