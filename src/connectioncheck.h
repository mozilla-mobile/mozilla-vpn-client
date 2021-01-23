/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONCHECK_H
#define CONNECTIONCHECK_H

#include "pinghelper.h"

// A simple class that uses pings to check the network status.
// It's used to check if the VPN connection succeeds.

class ConnectionCheck final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionCheck)

 public:
  ConnectionCheck();
  ~ConnectionCheck();

  void start();
  void stop();

 public slots:
  void pingSentAndReceived(qint64 msec);

 signals:
  void success();
  void failure();

 private:
  void timeout();

 private:
  QTimer m_timer;

  PingHelper m_pingHelper;
};

#endif  // CONNECTIONCHECK_H
