/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/counter.h"

#include <QDebug>
#include <QJsonValue>

#ifndef __wasm__
#  include "qtglean.h"
#endif

CounterMetric::CounterMetric(int id) : BaseMetric(id) {}

void CounterMetric::add(int amount) const {
#ifndef __wasm__
  return glean_counter_add(m_id, amount);
#else
  Q_UNUSED(amount);
#endif
}

int32_t CounterMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#ifndef __wasm__
  return glean_counter_test_get_num_recorded_errors(m_id, errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

QJsonValue CounterMetric::testGetValue(const QString& pingName) const {
#ifndef __wasm__
  return QJsonValue(glean_counter_test_get_value(m_id, pingName.toUtf8()));
#else
  Q_UNUSED(pingName);
  return QJsonValue(0);
#endif
}
