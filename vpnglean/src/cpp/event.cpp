/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/event.h"

#if not(defined(__wasm__) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>

EventMetric::EventMetric(int id, EventMetricExtraParser* parser)
    : m_id(id), m_parser(parser) {}

void EventMetric::record() const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  glean_event_record_no_extra(m_id);
#endif
}

void EventMetric::record(const QJsonObject& extras) {
  // Helper vector to extend the lifetime of the strings
  // that hold the extra key values until they are used.
  QList<QByteArray> keepStringsAlive;
  FfiExtra ffiExtras(extras.count(), extras.count());

  int count = 0;
  foreach (const QString& key, extras.keys()) {
    auto rawValue = extras.value(key);

    if (rawValue.isString()) {
      QByteArray value = rawValue.toString().toUtf8();
      keepStringsAlive.append(value);
      ffiExtras.values[count] = value.constData();
    } else if (rawValue.isBool()) {
      ffiExtras.values[count] = rawValue.toBool() ? "true" : "false";
    } else if (rawValue.isDouble()) {
      QByteArray value = QString::number(rawValue.toDouble()).toUtf8();
      keepStringsAlive.append(value);
      ffiExtras.values[count] = value.constData();
    } else {
      Q_ASSERT(false);
      // TODO: Record error.
      continue;
    }

    QByteArray k = key.toUtf8();
    keepStringsAlive.append(k);
    ffiExtras.keys[count] = k.constData();

    count++;
  }

  ffiExtras.values.resize(count);
  ffiExtras.keys.resize(count);
  if (!ffiExtras.keys.empty()) {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
    glean_event_record(m_id, ffiExtras.keys.data(), ffiExtras.values.data(),
                       static_cast<int32_t>(ffiExtras.keys.size()));
#endif
  } else {
    // As a standalone library, the Logger class isn't available. Use generic
    // logging instead.
    qWarning() << "Attempted to record an event with extras, but no extras "
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
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
    glean_event_record(m_id, ffiExtras.keys.data(), ffiExtras.values.data(),
                       static_cast<int32_t>(ffiExtras.keys.size()));
#endif
  } else {
    qWarning() << "Attempted to record an event with extras, but no extras "
                  "were provided. Ignoring.";
    // TODO: record an error.
  }
}

int32_t EventMetric::testGetNumRecordedErrors(ErrorType errorType) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_event_test_get_num_recorded_errors(m_id, errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

QList<QJsonObject> EventMetric::testGetValue(const QString& pingName) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
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
#else
  Q_UNUSED(pingName);
  return QList<QJsonObject>();
#endif
}
