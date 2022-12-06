/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENT_METRIC_H
#define EVENT_METRIC_H

#include "glean/glean.h"
#if not(defined(MVPN_WASM) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

struct FfiExtra {
  const char* const* keys;
  const char* const* values;
  int count;
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
  int __PRIVATE__id = 0;
};

struct EventMetricExtraParser {
  virtual FfiExtra fromJsonObject(const QJsonObject& extras,
                                  QList<QByteArray>& keepStringsAlive) {
    Q_ASSERT(false);
    // This function should be overriden.

    return FfiExtra();
  };
  virtual FfiExtra fromStruct(EventMetricExtra& extras,
                              QList<QByteArray>& keepStringsAlive) {
    Q_ASSERT(false);
    // This function should be overriden.

    return FfiExtra();
  }
};

class EventMetric final {
  Q_GADGET

 public:
  explicit EventMetric(
      int id, EventMetricExtraParser parser = EventMetricExtraParser());
  ~EventMetric() = default;

  Q_INVOKABLE void record() const;

  // This function should only be used from QML,
  // on C++ the variant that receives the FFI extra struct is preferred.
  Q_INVOKABLE void record(const QJsonObject& extras);

  void record(EventMetricExtra extras);

#if defined(UNIT_TEST)
  Q_INVOKABLE int32_t
  testGetNumRecordedErrors(VPNGlean::ErrorType errorType) const;

  Q_INVOKABLE QJsonArray testGetValue(const QString& pingName = "") const;
#endif

 private:
  int m_id;
  EventMetricExtraParser m_parser;
};

#endif  // EVENT_METRIC_H
