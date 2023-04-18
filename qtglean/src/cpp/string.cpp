/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/string.h"

#include <QDebug>

#ifndef __wasm__
#  include "qtglean.h"
#endif

StringMetric::StringMetric(int id) : m_id(id) {}

void StringMetric::set(QString value) const {
#ifndef __wasm__
  return glean_string_set(m_id, value.toLocal8Bit());
#endif
}

int32_t StringMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#ifndef __wasm__
  return glean_string_test_get_num_recorded_errors(m_id, errorType);
#endif
  return 0;
}

QString StringMetric::testGetValue(const QString& pingName) const {
#ifndef __wasm__
  return glean_string_test_get_value(m_id, pingName.toLocal8Bit());
#endif
  return "";
}
