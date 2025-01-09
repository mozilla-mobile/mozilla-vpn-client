/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/uuid.h"

#include <QDebug>
#include <QJsonValue>

#ifndef __wasm__
#  include "qtglean.h"
#endif

UuidMetric::UuidMetric(int id) : BaseMetric(id) {}

void UuidMetric::set(const QString& uuid) const {
#ifndef __wasm__
  return glean_uuid_set(m_id, uuid.toUtf8());
#endif
}

QString UuidMetric::generateAndSet() const {
#ifndef __wasm__
  return glean_uuid_generate_and_set(m_id);
#endif
  return QString();
}

int32_t UuidMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#ifndef __wasm__
  return glean_uuid_test_get_num_recorded_errors(m_id, errorType);
#endif
  return 0;
}

QJsonValue UuidMetric::testGetValue(const QString& pingName) const {
#ifndef __wasm__
  return QJsonValue(glean_uuid_test_get_value(m_id, pingName.toUtf8()));
#endif
  return QJsonValue("");
}
