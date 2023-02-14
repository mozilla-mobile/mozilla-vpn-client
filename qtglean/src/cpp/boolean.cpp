/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/boolean.h"

#include <QDebug>

#if not(defined(__wasm__) || defined(BUILD_QMAKE))
#  include "qtglean.h"
#endif

BooleanMetric::BooleanMetric(int id) : m_id(id) {}

void BooleanMetric::set(bool value) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_boolean_set(m_id, value);
#endif
}

int32_t BooleanMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_boolean_test_get_num_recorded_errors(m_id, errorType);
#endif
  return 0;
}

bool BooleanMetric::testGetValue(const QString& pingName) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_boolean_test_get_value(m_id, pingName.toLocal8Bit());
#endif
  return true;
}
