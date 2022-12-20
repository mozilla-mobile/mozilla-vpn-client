/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "glean/private/event.h"

#include "logger.h"
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif

#include <QJsonObject>
#include <QObject>
#include <QPair>
#if defined(UNIT_TEST)
#  include <QJsonDocument>
#endif

namespace {
Logger logger("VPNGlean::EventMetric");
}  // namespace

EventMetric::EventMetric(int id, EventMetricExtraParser* parser)
    : m_id(id), m_parser(parser) {}

void EventMetric::record() const {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_event_record_no_extra(m_id);
#endif
}

void EventMetric::record(const QJsonObject& extras) {
  // Helper vector to extend the lifetime of the strings
  // that hold the extra key values until they are used.
  QList<QByteArray> keepStringsAlive;
  FfiExtra ffiExtras = m_parser->fromJsonObject(extras, keepStringsAlive);

  if (!ffiExtras.keys.empty()) {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
    glean_event_record(m_id, ffiExtras.keys.data(), ffiExtras.values.data(),
                       ffiExtras.keys.size());
#endif
  } else {
    logger.error() << "Attempted to record an event with extras, but no extras "
                      "were provided. Ignoring.";
    // TODO: record an error.
  }
}

void EventMetric::record(const EventMetricExtra& extras) {
  // Helper vector to extend the lifetime of the strings
  // that hold the extra key values until they are used.
  QList<QByteArray> keepStringsAlive;
  FfiExtra ffiExtras = m_parser->fromStruct(extras, keepStringsAlive, m_id);

  if (!ffiExtras.keys.empty()) {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
    glean_event_record(m_id, ffiExtras.keys.data(), ffiExtras.values.data(),
                       ffiExtras.keys.size());
#endif
  } else {
    logger.error() << "Attempted to record an event with extras, but no extras "
                      "were provided. Ignoring.";
    // TODO: record an error.
  }
}

#if defined(UNIT_TEST)
int32_t EventMetric::testGetNumRecordedErrors(
    VPNGlean::ErrorType errorType) const {
#  if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  return glean_event_test_get_num_recorded_errors(
      m_id, static_cast<int32_t>(errorType));
#  else
  return 0;
#  endif
}

QJsonArray EventMetric::testGetValue(const QString& pingName) const {
#  if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  auto value = glean_event_test_get_value(m_id, pingName.toLocal8Bit());
  auto recordedEvents = QJsonDocument::fromJson(value).array();
  QList<QJsonObject> result;
  if (!recordedEvents.isEmpty()) {
    for (const QJsonValue& recordedEvent : recordedEvents) {
      Q_ASSERT(recordedEvent.isObject());
      result.append(recordedEvent.toObject());
    }
  }

  return result;
#  else
  return QList();
#  endif
}
#endif
