/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/private/event.h"
#include "vpnglean.h"
#if defined(UNIT_TEST)
#  include "glean/glean.h"
#endif

#include <QObject>
#include <QJsonObject>
#include <QPair>
#if defined(UNIT_TEST)
#  include <QJsonDocument>
#endif

#include <any>

EventMetric::EventMetric(int aId, EventMetricExtraParser aParser)
    : m_id(aId), m_parser(aParser) {}

void EventMetric::record() const { glean_event_record_no_extra(m_id); }

void EventMetric::record(const QJsonObject& extras) {
  FfiExtra ffiExtras = m_parser.fromJsonObject(extras, m_keepStringsAlive);
  if (ffiExtras.count > 0) {
    glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
  }
}

void EventMetric::record(EventMetricExtra extras) {
  Q_ASSERT(extras.__PRIVATE__id == m_id);

  FfiExtra ffiExtras = m_parser.fromStruct(extras, m_keepStringsAlive);
  if (ffiExtras.count > 0) {
    glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
  }
}

#if defined(UNIT_TEST)
int32_t EventMetric::testGetNumRecordedErrors(
    VPNGlean::ErrorType errorType) const {
  return glean_event_test_get_num_recorded_errors(
      m_id, static_cast<int32_t>(errorType));
}

QJsonArray EventMetric::testGetValue(const QString& pingName) const {
  auto value = glean_event_test_get_value(m_id, pingName.toLocal8Bit());
  QJsonArray recordedEvents = QJsonDocument::fromJson(value).array();
  if (!recordedEvents.isEmpty()) {
    for (const QJsonValue& recordedEvent : recordedEvents) {
      Q_ASSERT(recordedEvent.isObject());
    }
  }

  return recordedEvents;
}
#endif
