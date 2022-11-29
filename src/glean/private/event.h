/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENT_METRIC_H
#define EVENT_METRIC_H

#include "vpnglean.h"
#if defined(UNIT_TEST)
#  include "glean/glean.h"
#  include <QJsonArray>
#  include <QJsonDocument>
#endif

template <class T>
class EventMetric final {
 public:
  constexpr explicit EventMetric(int aId) : m_id(aId) {}

  void record() const { glean_event_record_no_extra(m_id); }

  void record(T extras) const {
    auto ffiExtras = extras.ToFfiExtra();
    glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
  }

#if defined(UNIT_TEST)
  int32_t testGetNumRecordedErrors(Glean::ErrorType errorType) const {
    return glean_event_test_get_num_recorded_errors(
        m_id, static_cast<int32_t>(errorType));
  }

  QJsonArray testGetValue(const QString& pingName = QString()) const {
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

 private:
  int m_id;
};

#endif  // EVENT_METRIC_H
