/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EXPONENTIALBACKOFFSTRATEGY_H
#define EXPONENTIALBACKOFFSTRATEGY_H

#include <QTimer>

constexpr const int BASE_RETRY_INTERVAL_SEC = 5;               // 5s
constexpr const int MAX_RETRY_INTERVAL_MSEC = 60 * 60 * 1000;  // 1hr

class ExponentialBackoffStrategy final : public QObject {
  Q_OBJECT
 public:
  ExponentialBackoffStrategy();
  int scheduleNextAttempt();
  void reset();

#ifdef UNIT_TEST
  void testOverrideBaseRetryInterval(int newInterval);
  void testOverrideMaxRetryInterval(int newInterval);
#endif

 signals:
  void executeNextAttempt();

 private:
  QTimer m_retryTimer;
  int m_retryCounter = 1;
  int m_maxInterval = MAX_RETRY_INTERVAL_MSEC;
  int m_baseInterval = BASE_RETRY_INTERVAL_SEC;
};

#endif  // EXPONENTIALBACKOFFSTRATEGY_H
