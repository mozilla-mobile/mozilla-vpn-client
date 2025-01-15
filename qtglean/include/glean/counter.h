/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COUNTER_H
#define COUNTER_H
#include <QObject>

#include "basemetric.h"
#include "errortype.h"

class CounterMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CounterMetric)

 public:
  explicit CounterMetric(int aId);

  Q_INVOKABLE void add(int amount = 1) const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // COUNTER_H
