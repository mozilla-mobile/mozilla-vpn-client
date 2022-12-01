/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENT_METRIC_H
#define EVENT_METRIC_H

#include "vpnglean.h"
#include "glean/glean.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <any>

struct FfiExtra {
  const char* const* keys;
  const char* const* values;
  int count;
};

struct EventMetricExtra {
  // This id is meant to be used to validate
  // a static cast of a specific extra struct into an `EventMetricExtra` struct.
  //
  // The extra structs cannot simply inherit `EventMetricExtra`,
  // because that prevents them from being initialized as an aggregate.
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

  virtual FfiExtra ToFfiExtra(
      QList<QPair<QByteArray, QByteArray>>& keepStringsAlive) {
    Q_ASSERT(false);

    // This function is meant to be overriden by the Glean generated code.
  }
};

class EventMetric final {
  Q_GADGET

 public:
  EventMetric(int aId, int extrasId = 0);
  ~EventMetric() = default;

  Q_INVOKABLE void record() const;

  // This function should only be used from QML,
  // on C++ the variant that receives the FFI extra struct is preferred.
  Q_INVOKABLE void record(const QJsonObject& extras);

  // For those wondering why on earth use std::any when we can just use a
  // template class: template classes cannot be used on classes exposed to QML.
  void record(std::any extras);

#if defined(UNIT_TEST)
  Q_INVOKABLE int32_t
  testGetNumRecordedErrors(Glean::ErrorType errorType) const;

  Q_INVOKABLE QJsonArray testGetValue(const QString& pingName = "") const;
#endif

 private:
  int m_id;
  int m_extrasId;

  // Helper vector to extend the lifetime of the strings
  // that hold the extra key values until they are used.
  QList<QPair<QByteArray, QByteArray>> m_keepStringsAlive;
};

#endif  // EVENT_METRIC_H
