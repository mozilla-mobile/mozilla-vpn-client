/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKIPFINDER_H
#define TASKIPFINDER_H

#include <QObject>

#include "taskscheduler/task.h"

class QHostAddress;
class QHostInfo;

class TaskIPFinder final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskIPFinder)

 public:
  TaskIPFinder();
  ~TaskIPFinder();

  void run() override;

 signals:
  void operationCompleted(const QString& ipv4, const QString& ipv6,
                          const QString& country);

 private slots:
  void dnsLookupCompleted(const QHostInfo& hostInfo);

 private:
  void createRequest(const QHostAddress& address, bool ipv6);
  void completeLookup();

 private:
  struct IPLookup {
    QString m_country;
    QString m_ipAddress;
    bool m_ipv6;
  };

  QList<IPLookup> m_results;
  uint32_t m_requestCount = 0;
  int m_lookupId = -1;
};

#endif  // TASKIPFINDER_H
