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

struct FfiExtra {
  const char* const* keys;
  const char* const* values;
  int count;
};

struct EventMetricExtra {
  virtual FfiExtra ToFfiExtra() {
    Q_ASSERT(false);

    // This function is meant to be overriden by the Glean generated code.
  }
};

class EventMetric final {
  Q_GADGET

 public:
  EventMetric(int aId);
  ~EventMetric() = default;

  Q_INVOKABLE void record() const;

  // This function should only be used from QML,
  // on C++ the variant that receives the FFI extra struct is preferred.
  //
  // Note: template classes cannot be used on methods exposed to QML.
  Q_INVOKABLE void record(QJsonObject extras);

  void record(EventMetricExtra extras) const;

#if defined(UNIT_TEST)
  Q_INVOKABLE int32_t
  testGetNumRecordedErrors(Glean::ErrorType errorType) const;

  Q_INVOKABLE QJsonArray testGetValue(const QString& pingName) const;
#endif

 private:
  int m_id;

  // Helper vector to extend the lifetime of the strings
  // that hold the extra key values until they are used.
  QVector<QPair<QByteArray, QByteArray>> m_keepQMLStringsAlive;
};

#endif  // EVENT_METRIC_H
