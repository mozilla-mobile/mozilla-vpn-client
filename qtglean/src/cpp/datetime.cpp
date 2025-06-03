/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/datetime.h"

#include <QDebug>
#include <QJsonValue>

#ifndef __wasm__
#  include "qtglean.h"
#endif

DatetimeMetric::DatetimeMetric(int id) : BaseMetric(id) {}

void DatetimeMetric::set() const {
#ifndef __wasm__
  return glean_datetime_set(m_id);
#endif
}

int32_t DatetimeMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#ifndef __wasm__
  return glean_datetime_test_get_num_recorded_errors(m_id, errorType);
#endif
  return 0;
}

QJsonValue DatetimeMetric::testGetValue(const QString& pingName) const {
#ifndef __wasm__
  auto value = glean_datetime_test_get_value_as_string(m_id, pingName.toUtf8());
  return QJsonValue(value);
#endif
  return QJsonValue("");
}
