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
    auto rawValue = extras.value(key);

    if (rawValue.isString()) {
      QByteArray value = rawValue.toString().toUtf8();
      m_keepStringsAlive.append(value);
      extraValues[count] = value.constData();
    } else if (rawValue.isBool()) {
      extraValues[count] = rawValue.toBool() ? "true" : "false";
    } else if (rawValue.isDouble()) {
      QByteArray value = QString::number(rawValue.toDouble()).toUtf8();
      extraValues[count] = value.constData();
    } else {
      Q_ASSERT(false);
      // TODO: Record error.
      continue;
    }

    QByteArray k = key.toUtf8();
    m_keepStringsAlive.append(k);
    extraKeys[count] = k.constData();

    count++;
  }

  glean_event_record(m_id, extraKeys, extraValues, count);
}

void EventMetric::record(EventMetricExtra extras) {
  // When calling record the specific extra will be statically cast to the
  // EventMetricExtra. We assert that the id is the same to be sure the correct
  // extra was used.
  Q_ASSERT(extras.__PRIVATE__id == m_extrasId);

  FfiExtra ffiExtras = extras.ToFfiExtra(m_keepStringsAlive);
  glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
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
