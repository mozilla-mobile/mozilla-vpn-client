/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BOOLEAN_H
#define BOOLEAN_H
#include <QObject>

#include "basemetric.h"
#include "errortype.h"

class BooleanMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BooleanMetric)

 public:
  explicit BooleanMetric(int aId);

  Q_INVOKABLE void set(bool value = true) const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // BOOLEAN_H
