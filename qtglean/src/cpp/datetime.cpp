/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/datetime.h"

#include <QDebug>

#ifndef __wasm__
#  include "qtglean.h"
#endif

DatetimeMetric::DatetimeMetric(int id) : m_id(id) {}

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

QString DatetimeMetric::testGetValueAsString(const QString& pingName) const {
#ifndef __wasm__
  return glean_datetime_test_get_value_as_string(m_id, pingName.toLocal8Bit());
#endif
  return "";
}

QDateTime DatetimeMetric::testGetValue(const QString& pingName) const {
  return QDateTime::fromString(testGetValueAsString(pingName),
                               Qt::ISODateWithMs);
}
