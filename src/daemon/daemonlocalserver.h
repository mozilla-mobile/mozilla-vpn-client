/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMONLOCALSERVER_H
#define DAEMONLOCALSERVER_H

#include <QLocalServer>

class DaemonLocalServer final : public QObject {
  Q_DISABLE_COPY_MOVE(DaemonLocalServer)

 public:
  explicit DaemonLocalServer(QString path, QObject* parent = nullptr);
  ~DaemonLocalServer();

  bool initialize();

 private:
  QString m_path;
  QLocalServer m_server;
};

#endif  // DAEMONLOCALSERVER_H
