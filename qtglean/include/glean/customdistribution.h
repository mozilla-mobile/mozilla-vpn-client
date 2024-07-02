/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CUSTOM_DISTRIBUTION_H
#define CUSTOM_DISTRIBUTION_H

#include <QHash>
#include <QObject>

#include "distributiondata.h"
#include "errortype.h"

class CustomDistributionMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CustomDistributionMetric)

 public:
  explicit CustomDistributionMetric(int aId);

  Q_INVOKABLE void accumulate(qint64 sample) const;

  // Test  only functions

  Q_INVOKABLE DistributionData testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // CUSTOM_DISTRIBUTION_H
