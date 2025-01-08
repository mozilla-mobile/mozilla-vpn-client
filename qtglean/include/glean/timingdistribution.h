/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIMING_DISTRIBUTION_H
#define TIMING_DISTRIBUTION_H

#include <QHash>
#include <QObject>

#include "basemetric.h"
#include "errortype.h"

class TimingDistributionMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TimingDistributionMetric)

 public:
  explicit TimingDistributionMetric(int aId);

  Q_INVOKABLE qint64 start() const;
  Q_INVOKABLE void stopAndAccumulate(qint64 timerId) const;
  Q_INVOKABLE void cancel(qint64 timerId) const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // TIMING_DISTRIBUTION_H
