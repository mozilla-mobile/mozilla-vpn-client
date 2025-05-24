/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/boolean.h"

#include <QDebug>
#include <QJsonValue>

#ifndef __wasm__
#  include "bindings/qtglean.h"
#endif

BooleanMetric::BooleanMetric(int id) : BaseMetric(id) {}

void BooleanMetric::set(bool value) const {
#ifndef __wasm__
  return glean_boolean_set(m_id, value);
#endif
}

int32_t BooleanMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#ifndef __wasm__
  return glean_boolean_test_get_num_recorded_errors(m_id, errorType);
#endif
  return 0;
}

QJsonValue BooleanMetric::testGetValue(const QString& pingName) const {
#ifndef __wasm__
  return QJsonValue(glean_boolean_test_get_value(m_id, pingName.toUtf8()));
#endif
  return QJsonValue(false);
}
