/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COUNTER_H
#define COUNTER_H
#include <QObject>

#include "errortype.h"

class CounterMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  CounterMetric() = default;
  CounterMetric(const CounterMetric&) = default;
  CounterMetric& operator=(const CounterMetric&) = default;

  explicit CounterMetric(int aId);
  ~CounterMetric() = default;

  Q_INVOKABLE void add(int amount = 1) const;

  // Test  only functions

  Q_INVOKABLE int32_t testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // COUNTER_H
