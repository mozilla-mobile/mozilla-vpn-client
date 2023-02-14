/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QUANTITY_H
#define QUANTITY_H
#include <QObject>

#include "errortype.h"

class QuantityMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  QuantityMetric() = default;
  QuantityMetric(const QuantityMetric&) = default;
  QuantityMetric& operator=(const QuantityMetric&) = default;

  explicit QuantityMetric(int aId);
  ~QuantityMetric() = default;

  Q_INVOKABLE void set(int value = 0) const;

  // Test  only functions

  Q_INVOKABLE int64_t testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // QUANTITY_H