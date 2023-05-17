/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/counter.h"

#include <QDebug>

#if not(defined(__wasm__) || defined(BUILD_QMAKE))
#  include "qtglean.h"
#endif

CounterMetric::CounterMetric(int id) : m_id(id) {}

void CounterMetric::add(int amount) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_counter_add(m_id, amount);
#else
  Q_UNUSED(amount);
#endif
}

int32_t CounterMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_counter_test_get_num_recorded_errors(m_id, errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

int32_t CounterMetric::testGetValue(const QString& pingName) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_counter_test_get_value(m_id, pingName.toUtf8())
#else
  Q_UNUSED(pingName);
  return 0;
#endif
}
