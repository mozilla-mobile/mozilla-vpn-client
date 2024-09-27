/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MEMORY_DISTRIBUTION_H
#define MEMORY_DISTRIBUTION_H

#include <QHash>
#include <QObject>

#include "distributiondata.h"
#include "errortype.h"

// !!!IMPORTANT!!!
// All work for memory_distribution metric type was done as part of this
// PR: https://github.com/mozilla-mobile/mozilla-vpn-client/pull/9318
// After it was written, it was decided to use a custom_distribution
// data type instead.
// This memory distribution work has NOT been tested, though the code has
// stayed in so that this work is not lost, in the event we want to use
// this data type in the future.

class MemoryDistributionMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MemoryDistributionMetric)

 public:
  explicit MemoryDistributionMetric(int aId);

  Q_INVOKABLE void accumulate(qint64 sample) const;

  // Test  only functions

  Q_INVOKABLE DistributionData testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // MEMORY_DISTRIBUTION_H
