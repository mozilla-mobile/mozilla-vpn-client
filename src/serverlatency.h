/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERLATENCY_H
#define SERVERLATENCY_H

#include "pingsender.h"
#include "task.h"

#include <QObject>
#include <QTimer>

class ServerLatency final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerLatency)

 public:
  ServerLatency();
  ~ServerLatency();

  void initialize();
  void start();
  void stop();

 private:
  void maybeSendPings();

 private:
  struct ServerPingRecord {
    QString publicKey;
    quint64 timestamp;
    quint16 sequence;
    int retries;
  };
  quint16 m_sequence = 0;
  PingSender* m_pingSender = nullptr;
  QList<QString> m_pingSendQueue;
  QList<ServerPingRecord> m_pingReplyList;

  QTimer m_pingTimeout;
  QTimer m_refreshTimer;
  bool m_wantRefresh = false;

 private slots:
  void stateChanged();
  void recvPing(quint16 sequence);
  void criticalPingError();
};

#endif  // SERVERLATENCY_H
