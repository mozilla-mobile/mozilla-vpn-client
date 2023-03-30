/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QUANTITY_H
#define QUANTITY_H

#include <QObject>

#include "errortype.h"

class QuantityMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(QuantityMetric)

 public:
  explicit QuantityMetric(int aId);

  Q_INVOKABLE void set(int value = 0) const;

  // Test  only functions

  Q_INVOKABLE int64_t testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // QUANTITY_H
