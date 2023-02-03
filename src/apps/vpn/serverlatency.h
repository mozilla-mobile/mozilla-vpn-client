/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERLATENCY_H
#define SERVERLATENCY_H

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include "pingsender.h"
#include "task.h"

class ServerLatency final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerLatency)

  Q_PROPERTY(QDateTime lastUpdateTime READ lastUpdateTime CONSTANT)
  Q_PROPERTY(unsigned int avgLatency READ avgLatency CONSTANT)

 public:
  ServerLatency();
  ~ServerLatency();

  unsigned int avgLatency() const;
  unsigned int getLatency(const QString& pubkey) const {
    return m_latency.value(pubkey);
  };
  const QDateTime& lastUpdateTime() const { return m_lastUpdateTime; }

  void initialize();
  void start();
  void stop();

  Q_INVOKABLE void refresh();

 private:
  void maybeSendPings();

 private:
  struct ServerPingRecord {
    QString publicKey;
    quint64 timestamp;
    quint16 sequence;
    double distance;
    int retries;
  };
  quint16 m_sequence = 0;
  PingSender* m_pingSender = nullptr;
  QList<ServerPingRecord> m_pingSendQueue;
  QList<ServerPingRecord> m_pingReplyList;

  QHash<QString, qint64> m_latency;
  qint64 m_sumLatencyMsec = 0;
  QDateTime m_lastUpdateTime;

  QTimer m_pingTimeout;
  QTimer m_refreshTimer;
  bool m_wantRefresh = false;

 private slots:
  void stateChanged();
  void recvPing(quint16 sequence);
  void criticalPingError();
};

#endif  // SERVERLATENCY_H
