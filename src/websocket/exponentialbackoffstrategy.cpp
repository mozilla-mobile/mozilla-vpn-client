/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "exponentialbackoffstrategy.h"
#include "leakdetector.h"

#include <QtMath>

ExponentialBackoffStrategy::ExponentialBackoffStrategy() {
  MVPN_COUNT_CTOR(ExponentialBackoffStrategy);

  connect(&m_retryTimer, &QTimer::timeout, this,
          &ExponentialBackoffStrategy::executeNextAttempt);
  m_retryTimer.setSingleShot(true);
}

ExponentialBackoffStrategy::~ExponentialBackoffStrategy() {
  MVPN_COUNT_DTOR(ExponentialBackoffStrategy);
}

#ifdef UNIT_TEST
void ExponentialBackoffStrategy::testOverrideBaseRetryInterval(
    int newInterval) {
  m_baseInterval = newInterval;
}

void ExponentialBackoffStrategy::testOverrideMaxRetryInterval(int newInterval) {
  m_maxInterval = newInterval;
}
#endif

/**
 * @brief Schedules the next attempt to execute a given function.
 *
 * Everytime a new attempt is scheduled the interval will be exponentially
 * larger.
 *
 * @returns The interval until the next attempt is executed, in milliseconds.
 */
int ExponentialBackoffStrategy::scheduleNextAttempt() {
  int retryInterval = qPow(m_baseInterval, m_retryCounter);
// Outside of tests we assume m_baseInterval is in seconds, not
// milliseconds. In testing mode, that would make wait times too long,
// se we assume intervals are in milliseconds and no transformation is needed.
#ifndef UNIT_TEST
  retryInterval *= 1000;
#endif
  if (retryInterval < m_maxInterval) {
    m_retryCounter++;
  }
  m_retryTimer.start(retryInterval);
  return retryInterval;
}

/**
 * @brief Stops the timer for an ongoing execution attempt, if any. Resets the
 * interval to base interval.
 */
void ExponentialBackoffStrategy::reset() {
  m_retryTimer.stop();
  m_retryCounter = 1;
}
