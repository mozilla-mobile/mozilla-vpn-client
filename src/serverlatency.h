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
  struct ServerPingRecord {
    QString publicKey;
    quint64 timestamp;
  };
  PingSender* m_pingSender = nullptr;
  QMap<int, ServerPingRecord> m_pingReplyList;
  QTimer m_timeout;
  bool m_wantRefresh = false;

 private slots:
  void stateChanged();
  void recvPing(quint16 sequence);
  void criticalPingError();
};

#endif  // SERVERLATENCY_H
