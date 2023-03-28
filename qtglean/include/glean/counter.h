/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COUNTER_H
#define COUNTER_H
#include <QObject>

#include "errortype.h"

class CounterMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CounterMetric)

 public:
  explicit CounterMetric(int aId);

  Q_INVOKABLE void add(int amount = 1) const;

  // Test  only functions

  Q_INVOKABLE int32_t testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // COUNTER_H
