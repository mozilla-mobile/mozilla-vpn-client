/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CUSTOM_DISTRIBUTION_H
#define CUSTOM_DISTRIBUTION_H

#include <QHash>
#include <QObject>

#include "basemetric.h"
#include "errortype.h"

class CustomDistributionMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CustomDistributionMetric)

 public:
  explicit CustomDistributionMetric(int aId);

  Q_INVOKABLE void accumulate_single_sample(qint64 sample) const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // CUSTOM_DISTRIBUTION_H
