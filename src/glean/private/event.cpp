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

EventMetric::EventMetric(int aId, int extrasId)
    : m_id(aId), m_extrasId(extrasId) {}

void EventMetric::record() const { glean_event_record_no_extra(m_id); }

void EventMetric::record(const QJsonObject& extras) {
  const char* extraValues[extras.size()];
  const char* extraKeys[extras.size()];
  int count = 0;

  m_keepStringsAlive.clear();

  foreach (const QString& key, extras.keys()) {
    auto value = extras.value(key).toString();
    if (!value.isNull()) {
      QByteArray k = key.toUtf8();
      QByteArray v = value.toUtf8();
      m_keepStringsAlive.append(QPair(k, v));

      extraValues[count] = v.constData();
      extraKeys[count] = k.constData();
      count++;
    }
  }

  glean_event_record(m_id, extraKeys, extraValues, count);
}

void EventMetric::record(std::any extras) {
  EventMetricExtra parsedExtras = std::any_cast<EventMetricExtra>(extras);
  Q_ASSERT(parsedExtras.__PRIVATE__id == m_extrasId);

  FfiExtra ffiExtras = parsedExtras.ToFfiExtra(m_keepStringsAlive);
  glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
}

#if defined(UNIT_TEST)
int32_t EventMetric::testGetNumRecordedErrors(
    Glean::ErrorType errorType) const {
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
