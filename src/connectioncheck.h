/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONCHECK_H
#define CONNECTIONCHECK_H

#include <QObject>
#include <QTimer>

class NetworkRequest;

// A simple class that does 1 network request with X seconds of timeout.
// It's used to check if the VPN connection succeeds.

class ConnectionCheck final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionCheck)

 public:
  ConnectionCheck();
  ~ConnectionCheck();

  void start();
  void stop();

 signals:
  void success();
  void failure();

 private:
  void startInternal();
  void timeout();
  void maybeTryAgain();

 private:
  QTimer m_timer;

  NetworkRequest* m_networkRequest = nullptr;

  uint32_t m_step = 0;
};

#endif  // CONNECTIONCHECK_H
