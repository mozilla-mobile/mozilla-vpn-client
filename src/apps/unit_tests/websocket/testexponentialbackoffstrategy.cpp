/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.orgß/MPL/2.0/. */

#include "testexponentialbackoffstrategy.h"

#include <QtMath>

#include "helper.h"
#include "settingsholder.h"
#include "websocket/exponentialbackoffstrategy.h"

void TestExponentialBackoffStrategy::
    tst_reconnectionBackoffTimeExponentiallyIncreases() {
  ExponentialBackoffStrategy backoffStrategy;

  int testBaseRetryInterval = 5;
  int testMaxRetries = 3;
  backoffStrategy.testOverrideBaseRetryInterval(testBaseRetryInterval);
  backoffStrategy.testOverrideMaxRetryInterval(
      qPow(testBaseRetryInterval, testMaxRetries));

  int callCount = 0;
  connect(&backoffStrategy, &ExponentialBackoffStrategy::executeNextAttempt,
          [&callCount]() { callCount++; });

  for (int i = 0; i < testMaxRetries; i++) {
    // Schedule an attempt.
    int nextAttemptIn = backoffStrategy.scheduleNextAttempt();
    // Verify interval is the expected value.
    QCOMPARE(nextAttemptIn, qPow(testBaseRetryInterval, i + 1));
    // `testFn` should only have been scheduled at this point, not called.
    QCOMPARE(callCount, i);
    // Wait for testFn to be executed.
    QVERIFY(
        QTest::qWaitFor([&callCount, i]() { return callCount == (i + 1); }));
  }

  // Inside the loop we have reached max retries, so we expect the interval to
  // be the same as the last one now.

  // Schedule an attempt.
  int nextAttemptIn = backoffStrategy.scheduleNextAttempt();
  // Verify interval is the expected value.
  QCOMPARE(nextAttemptIn, qPow(testBaseRetryInterval, testMaxRetries));
  // `testFn` should only have been scheduled at this point, not called.
  QCOMPARE(callCount, testMaxRetries);
  // Wait for testFn to be executed.
  QVERIFY(QTest::qWaitFor([&callCount, testMaxRetries]() {
    return callCount == testMaxRetries + 1;
  }));

  // After a reset, the interval should be back to base interval.
  backoffStrategy.reset();

  // Schedule an attempt.
  nextAttemptIn = backoffStrategy.scheduleNextAttempt();
  // Verify interval is the expected value.
  QCOMPARE(nextAttemptIn, testBaseRetryInterval);
  // `testFn` should only have been scheduled at this point, not called.
  QCOMPARE(callCount, testMaxRetries + 1);
  // Wait for testFn to be executed.
  QVERIFY(QTest::qWaitFor([&callCount, testMaxRetries]() {
    return callCount == testMaxRetries + 2;
  }));
}

static TestExponentialBackoffStrategy s_testExponentialBackoffStrategy;
