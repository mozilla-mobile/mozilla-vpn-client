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

  Q_PROPERTY(
      QDateTime lastUpdateTime READ lastUpdateTime NOTIFY progressChanged)
  Q_PROPERTY(unsigned int avgLatency READ avgLatency NOTIFY progressChanged)
  Q_PROPERTY(bool isActive READ isActive NOTIFY progressChanged)
  Q_PROPERTY(double progress READ progress NOTIFY progressChanged)

 public:
  ServerLatency();
  ~ServerLatency();

  unsigned int avgLatency() const;
  unsigned int getLatency(const QString& pubkey) const {
    return m_latency.value(pubkey);
  };
  const QDateTime& lastUpdateTime() const { return m_lastUpdateTime; }
  bool isActive() const { return m_pingSender != nullptr; }
  double progress() const;

  qint64 getCooldown(const QString& pubkey) const {
    return m_cooldown.value(pubkey);
  }
  void setCooldown(const QString& pubkey, qint64 timeout);

  void initialize();
  void start();
  void stop();

  Q_INVOKABLE void refresh();

 signals:
  void progressChanged();

 private:
  void maybeSendPings();
  void clear();

 private:
  struct ServerPingRecord {
    QString publicKey;
    QString countryCode;
    QString cityName;
    quint64 timestamp;
    quint16 sequence;
    double distance;
    int retries;
  };
  quint16 m_sequence = 0;
  PingSender* m_pingSender = nullptr;
  QList<ServerPingRecord> m_pingSendQueue;
  QList<ServerPingRecord> m_pingReplyList;
  qsizetype m_pingSendTotal = 0;

  QHash<QString, qint64> m_latency;
  QHash<QString, qint64> m_cooldown;
  qint64 m_sumLatencyMsec = 0;
  QDateTime m_lastUpdateTime;

  QTimer m_pingTimeout;
  QTimer m_refreshTimer;
  QTimer m_progressDelayTimer;
  bool m_wantRefresh = false;

 private slots:
  void stateChanged();
  void recvPing(quint16 sequence);
  void criticalPingError();
};

#endif  // SERVERLATENCY_H
