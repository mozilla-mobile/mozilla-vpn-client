/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENT_METRIC_H
#define EVENT_METRIC_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "errortype.h"

struct FfiExtra {
  std::vector<const char*> keys;
  std::vector<const char*> values;

  FfiExtra() : keys(0), values(0) {}

  FfiExtra(int keysSize, int valuesSize) : keys(keysSize), values(valuesSize) {}
};

struct EventMetricExtra {
  // This id is meant to be used to validate
  // a static cast of a specific extra struct into an `EventMetricExtra` struct.
  //
  // We need to static cast, because template classes cannot be annotated with
  // Q_GADGET or Q_OBJECT like the EventMetric class needs to be, so we have to
  // have this generic class to use as an argument for `record()`.
  //
  // Aggregate initialization is also not available for structs with private
  // fields, so we stick to this ugly __PRIVATE__ prefix.
  //
  // See: https://en.cppreference.com/w/cpp/language/aggregate_initialization
  //
  // Aggregate initialization is preferred here because it allows the MozillaVPN
  // Glean APIs to be exactly the same as the Firefox Desktop Glean APIs. Also
  // and probably most importantly, it's also just looks better to have a key
  // value initialization in this case since all extras are optional.
  int __PRIVATE__id;
};

struct EventMetricExtraParser {
  virtual FfiExtra fromStruct(const EventMetricExtra& extras,
                              QList<QByteArray>& keepStringsAlive, int id) {
    Q_UNUSED(extras);
    Q_UNUSED(keepStringsAlive);
    Q_UNUSED(id);
    Q_ASSERT(false);
    // This function should be overriden.

    return FfiExtra();
  }
};

class EventMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  EventMetric() = default;
  EventMetric(const EventMetric&) = default;
  EventMetric& operator=(const EventMetric&) = default;

  explicit EventMetric(
      int id, EventMetricExtraParser* parser = new EventMetricExtraParser());
  ~EventMetric() = default;

  Q_INVOKABLE void record() const;

  // This function should only be used from QML,
  // on C++ the variant that receives the FFI extra struct is preferred.
  Q_INVOKABLE void record(const QJsonObject& extras);

  void record(const EventMetricExtra& extras);

  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

  Q_INVOKABLE QList<QJsonObject> testGetValue(
      const QString& pingName = "") const;

 private:
  int m_id;
  EventMetricExtraParser* m_parser;
};

#endif  // EVENT_METRIC_H
