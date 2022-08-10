/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKLATENCY_H
#define TASKLATENCY_H

#include "pingsender.h"
#include "task.h"

#include <QObject>
#include <QTimer>

class TaskLatency final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskLatency)

 public:
  TaskLatency();
  ~TaskLatency();

  void run() override;

 private:
  struct ServerPingRecord {
    QString publicKey;
    quint64 timestamp;
  };
  PingSender* m_pingSender = nullptr;
  QMap<int, ServerPingRecord> m_pingReplyList;
  QTimer m_timeout;

 private slots:
  void recvPing(quint16 sequence);
  void criticalPingError();
};

#endif  // TASKLATENCY_H
