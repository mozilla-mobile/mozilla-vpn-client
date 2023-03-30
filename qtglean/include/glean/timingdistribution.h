/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIMING_DISTRIBUTION_H
#define TIMING_DISTRIBUTION_H

#include <QHash>
#include <QObject>

#include "errortype.h"

// Based on Glean's DistributionData struct.
// https://github.com/mozilla/glean/blob/main/glean-core/src/metrics/mod.rs#L80
//
// A snapshot of all buckets and the accumulated sum of a distribution.
struct DistributionData {
  // A map containig the bucket index mapped to the accumulated count.
  //
  // This can contain buckets with a count of `0`.
  QHash<int, int> values;
  // The accumulated sum of all the samples in the distribution.
  int sum;
  // The total number of entries in the distribution.
  int count;

  DistributionData() : sum(0), count(0) {}
};

class TimingDistributionMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TimingDistributionMetric)

 public:
  explicit TimingDistributionMetric(int aId);

  Q_INVOKABLE int start() const;
  Q_INVOKABLE void stopAndAccumulate(int timerId) const;
  Q_INVOKABLE void cancel(int timerId) const;

  // Test  only functions

  Q_INVOKABLE DistributionData testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // TIMING_DISTRIBUTION_H
